#include "TitleParser.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <libxml++/libxml++.h>

std::vector<TitleParser::TitleInfo> TitleParser::getTitleInfos(const std::shared_ptr<FileDevice>& device, std::vector<uint8_t>& key)
{
    std::vector<TitleParser::TitleInfo> infos;

    for (unsigned int i = 0; i < pathsSize; i++)
    {
        std::shared_ptr<Directory> dir = Wfs(device, key).GetDirectory(titlePaths[i]);
        if (dir)
        {
            for (auto item : *dir) 
            {
                if (item->IsDirectory())
                {
                    boost::filesystem::path npath = boost::filesystem::path(titlePaths[i]) / item->GetRealName();
                    std::shared_ptr<Directory> titleDir = Wfs(device, key).GetDirectory(npath.string());
                    std::shared_ptr<Directory> metaDir = titleDir->GetDirectory("meta");
                    if (metaDir && metaDir->GetItemsCount() > 0)
                    {
                        TitleInfo info;
                        info.name = "";
                        info.titleId = "";

                        std::shared_ptr<File> metaFile = metaDir->GetFile("meta.xml");
                        if (metaFile)
                        {
                            size_t size = metaFile->GetSize();
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

                        std::shared_ptr<File> iconFile = metaDir->GetFile("icontex.tga");
                        if (iconFile)
                        {
                            size_t size = iconFile->GetSize();
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
        }
    }

    return infos;
}

TitleParser::AdditionalInfo TitleParser::getAdditionalInfo(const std::string& titleId, const std::shared_ptr<FileDevice>& device, std::vector<uint8_t>& key)
{
    AdditionalInfo info;
    info.gamePath = std::string("/usr/title/") + boost::algorithm::to_lower_copy(titleId.substr(0, 8)) + std::string("/") + boost::algorithm::to_lower_copy(titleId.substr(8, 8));
    info.dlcPath = "";
    info.updatePath = "";
    info.updateVersion = "";
    info.shortTid = titleId.substr(8, 16);

    boost::filesystem::path dlcPath = boost::filesystem::path("/usr/title/0005000c") / boost::algorithm::to_lower_copy(titleId.substr(8, 8));
    std::shared_ptr<Directory> dlcDir = Wfs(device, key).GetDirectory(dlcPath.string());
    if (dlcDir && dlcDir->GetItemsCount() > 0)
    {
        info.dlcPath = dlcPath.string();
    }

    boost::filesystem::path updatePath = boost::filesystem::path("/usr/title/0005000e") / boost::algorithm::to_lower_copy(titleId.substr(8, 8));
    std::shared_ptr<Directory> updateDir = Wfs(device, key).GetDirectory(updatePath.string());
    if (updateDir && updateDir->GetItemsCount() > 0)
    {
        info.updatePath = updatePath.string();

        std::shared_ptr<Directory> metaDir = updateDir->GetDirectory("meta");
        if (metaDir && metaDir->GetItemsCount() > 0)
        {
            std::shared_ptr<File> metaFile = metaDir->GetFile("meta.xml");
            if (metaFile)
            {
                size_t size = metaFile->GetSize();
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

        for (auto item : *saveDir) 
        {
            users.push_back(item->GetRealName());
        }
    }
    info.users = users;

    return info;
}