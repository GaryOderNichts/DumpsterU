#pragma once

#include <string>
#include <vector>

class DiskUtils
{
public:
    struct Disk
    {
        std::string deviceId;
        std::string name;
        std::string size;
    };

    static std::vector<DiskUtils::Disk> getDisks();
};