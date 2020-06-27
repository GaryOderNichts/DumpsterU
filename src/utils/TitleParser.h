/**    
 *  Copyright (C) 2020 GaryOderNichts
 *  This file is part of DumpsterU <https://github.com/GaryOderNichts/DumpsterU>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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