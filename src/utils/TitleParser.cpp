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

#include "TitleParser.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <libxml++/libxml++.h>

std::vector<TitleParser::TitleInfo> TitleParser::getTitleInfos(const std::shared_ptr<FileDevice>& device, std::vector<std::byte>& key)
{
    std::vector<TitleParser::TitleInfo> infos;

    for (unsigned int i = 0; i < pathsSize; i++)
    {
        std::shared_ptr<Directory> dir = nullptr;
        try
        {
            dir = Wfs(device, key).GetDirectory(titlePaths[i]);
        }
        catch(const WfsException& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            continue;
        }

        if (dir)
        {
            for (auto [name, item_or_error] : *dir) 
            {
                try
                {
                    auto item = throw_if_error(item_or_error);
                    if (item->IsDirectory())
                    {
                        boost::filesystem::path npath = boost::filesystem::path(titlePaths[i]) / name;
                        std::shared_ptr<Directory> titleDir = Wfs(device, key).GetDirectory(npath.string());
                        std::shared_ptr<Directory> metaDir = throw_if_error(titleDir->GetDirectory("meta"));
                        if (metaDir)
                        {
                            TitleInfo info;
                            info.name = "";
                            info.titleId = "";

                            std::shared_ptr<File> metaFile = throw_if_error(metaDir->GetFile("meta.xml"));
                            if (metaFile)
                            {
                                size_t size = metaFile->Size();
                                if (size > 0)
                                {
                                    File::stream stream(metaFile);
                                    xmlpp::DomParser parser;
                                    parser.parse_stream(stream);

                                    xmlpp::Node* rootNode = parser.get_document()->get_root_node();

                                    xmlpp::Node::NodeSet nameResult = rootNode->find("longname_en");
                                    if (nameResult.size() > 0)
                                    {
                                        xmlpp::Element* longname = (xmlpp::Element*) nameResult.at(0);
                                        info.name = longname->get_first_child_text()->get_content();
                                    }

                                    xmlpp::Node::NodeSet titleIdResult = rootNode->find("title_id");
                                    if (titleIdResult.size() > 0)
                                    {
                                        xmlpp::Element* titleId = (xmlpp::Element*) titleIdResult.at(0);
                                        info.titleId = titleId->get_first_child_text()->get_content();
                                    }

                                }
                            }

                            std::shared_ptr<File> iconFile = throw_if_error(metaDir->GetFile("icontex.tga"));
                            if (iconFile)
                            {
                                size_t size = iconFile->Size();
                                if (size > 0)
                                {
                                    File::stream stream(iconFile);
                                    void* data = malloc(size);
                                    stream.read((char*) data, size);

                                    Glib::RefPtr<Gio::MemoryInputStream> gstream = Gio::MemoryInputStream::create();
                                    gstream->add_data(data, size);
                                    info.icon = Gdk::Pixbuf::create_from_stream_at_scale(gstream, 72, 72, false); // 72x72 is the highest resolution we ever need in this application

                                    free(data);
                                }
                            }
                            else
                            {
                                std::cerr << "Cannot get icon " << npath << " \n";
                            }
                            
                            infos.push_back(info);
                        }
                    }
                }
                catch(const WfsException& e)
                {
                    std::cerr << "Error: " << e.what() << std::endl;
                }
            }
        }
    }

    return infos;
}

TitleParser::AdditionalInfo TitleParser::getAdditionalInfo(const std::string& titleId, const std::shared_ptr<FileDevice>& device, std::vector<std::byte>& key)
{
    AdditionalInfo info;
    info.gamePath = std::string("/usr/title/") + boost::algorithm::to_lower_copy(titleId.substr(0, 8)) + std::string("/") + boost::algorithm::to_lower_copy(titleId.substr(8, 8));
    info.dlcPath = "";
    info.updatePath = "";
    info.updateVersion = "";
    info.shortTid = titleId.substr(8, 16);

    try
    {
        boost::filesystem::path dlcPath = boost::filesystem::path("/usr/title/0005000c") / boost::algorithm::to_lower_copy(titleId.substr(8, 8));
        std::shared_ptr<Directory> dlcDir = Wfs(device, key).GetDirectory(dlcPath.string());
        if (dlcDir)
        {
            info.dlcPath = dlcPath.string();
        }

        boost::filesystem::path updatePath = boost::filesystem::path("/usr/title/0005000e") / boost::algorithm::to_lower_copy(titleId.substr(8, 8));
        std::shared_ptr<Directory> updateDir = Wfs(device, key).GetDirectory(updatePath.string());
        if (updateDir)
        {
            info.updatePath = updatePath.string();

            std::shared_ptr<Directory> metaDir = throw_if_error(updateDir->GetDirectory("meta"));
            if (metaDir)
            {
                std::shared_ptr<File> metaFile = throw_if_error(metaDir->GetFile("meta.xml"));
                if (metaFile)
                {
                    size_t size = metaFile->Size();
                    if (size > 0)
                    {
                        File::stream stream(metaFile);
                        xmlpp::DomParser parser;
                        parser.parse_stream(stream);

                        xmlpp::Node* rootNode = parser.get_document()->get_root_node();

                        xmlpp::Node::NodeSet versionResult = rootNode->find("title_version");
                        if (versionResult.size() > 0)
                        {
                            xmlpp::Element* version = (xmlpp::Element*) versionResult.at(0);
                            info.updateVersion = version->get_first_child_text()->get_content();
                        }
                    }
                }
            }
        }

        std::vector<std::string> users;
        boost::filesystem::path savePath = boost::filesystem::path("/usr/save/00050000") / boost::filesystem::path(boost::algorithm::to_lower_copy(titleId.substr(8, 8))) / "user";
        std::shared_ptr<Directory> saveDir = Wfs(device, key).GetDirectory(savePath.string());
        if (saveDir)
        {
            info.savePath = savePath.string();

            for (auto [name, item_or_error] : *saveDir) 
            {
                if (item_or_error.has_value()) {
                    users.push_back(name);
                }
            }
        }
        info.users = users;
    }
    catch(const WfsException& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return info;
}