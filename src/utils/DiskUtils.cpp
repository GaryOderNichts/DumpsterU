#include "DiskUtils.h"

#include <iostream>
#include <filesystem>

std::vector<std::string> DiskUtils::getDisks()
{
    std::vector<std::string> disks;
#ifdef _WIN32
    // TODO detect win drives
#else
    for (const auto & entry : std::filesystem::directory_iterator("/sys/block"))
    {
        std::string filename = entry.path().filename();

        if (filename.find("loop", 0) == std::string::npos)
        {
            disks.push_back(filename);
        }
    }
#endif

    return disks;
}