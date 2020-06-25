#include "DiskUtils.h"

#include <iostream>
#include <filesystem>

#ifdef _WIN32
    #include <comdef.h>
    #include <Wbemidl.h>

    // Required on windows for wstring conversion
    #include <locale> 
    #include <codecvt>
#endif

std::vector<DiskUtils::Disk> DiskUtils::getDisks()
{
    std::vector<Disk> disks;
#ifdef _WIN32
    HRESULT hres;
    hres =  CoInitializeEx(0, COINIT_MULTITHREADED);
    hres =  CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *) &pLoc);

    IWbemServices *pSvc = NULL;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, 0, 0, 0, &pSvc);

    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(_bstr_t(L"WQL"), _bstr_t(L"SELECT * FROM Win32_DiskDrive"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;
   
    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if(uReturn == 0)
            break;

        Disk disk;
        using convert_type = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_type, wchar_t> converter;

        VARIANT vtProp;

        hr = pclsObj->Get(L"Partitions", 0, &vtProp, 0, 0);
        if (vtProp.uintVal != 0) // Wii U Drives show up without any partitions
        {
            continue;
            VariantClear(&vtProp);
        }
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"DeviceID", 0, &vtProp, 0, 0);
        std::wstring ws(vtProp.bstrVal);
        disk.deviceId = converter.to_bytes(ws);
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"Caption", 0, &vtProp, 0, 0);
        ws = std::wstring(vtProp.bstrVal);
        disk.name = converter.to_bytes(ws).substr(0, 20);
        VariantClear(&vtProp);
        if (disk.name.empty())
        {
            disk.name = disk.deviceId;
        }

        hr = pclsObj->Get(L"Size", 0, &vtProp, 0, 0);
        ws = std::wstring(vtProp.bstrVal);
        uint64_t size = std::stoull(converter.to_bytes(ws));
        disk.size = std::to_string((int) (size / 1000 / 1000 / 1000)) + " GiB";
        VariantClear(&vtProp);

        disks.push_back(disk);

        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
#else
    for (const auto & entry : std::filesystem::directory_iterator("/sys/block"))
    {
        std::string filename = entry.path().filename();

        if (filename.find("loop", 0) == std::string::npos)
        {
            Disk disk;
            disk.name = filename;
            disk.deviceId = std::string("/dev/") + filename;
            disks.push_back(disk);
        }
    }
#endif

    return disks;
}