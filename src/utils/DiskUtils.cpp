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
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);

    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    IEnumWbemClassObject* pEnumerator = NULL;
    BSTR bstr_wql = SysAllocString(L"WQL");
    BSTR bstr_sql = SysAllocString(L"SELECT * FROM Win32_DiskDrive");
    hres = pSvc->ExecQuery(bstr_wql, bstr_sql, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;
   
    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if(uReturn == 0)
            break;

        VARIANT vtProp;
        hr = pclsObj->Get(L"DeviceID", 0, &vtProp, 0, 0);

        std::wstring ws(vtProp.bstrVal);

        using convert_type = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_type, wchar_t> converter;
        std::string str = converter.to_bytes(ws);

        disks.push_back(str);

        VariantClear(&vtProp);

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