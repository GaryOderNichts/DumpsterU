#pragma once

#include <string>
#include <vector>
#include <map>
#include <gtkmm.h>
#include "wfslib/WfsLib.h"

class TitleParser
{
public:
    struct TitleInfo
    {
        std::string name;
        std::string titleId;
        Glib::RefPtr<Gdk::Pixbuf> icon;
    };
    
    struct AdditionalInfo
    {
        std::string gamePath;
        std::string dlcPath;
        std::string updatePath;
        std::string updateVersion;
        std::string savePath;
        std::vector<std::string> users;
        std::string shortTid;
    };

    static std::vector<TitleParser::TitleInfo> getTitleInfos(const std::shared_ptr<FileDevice>& device, std::vector<uint8_t>& key);
    static TitleParser::AdditionalInfo getAdditionalInfo(const std::string& titleId, const std::shared_ptr<FileDevice>& device, std::vector<uint8_t>& key);

private:
    static const unsigned int pathsSize = 2;
    static constexpr const char* titlePaths[] = 
    {
        "/usr/title/00050000",
        "/usr/title/00050002",
    };
};