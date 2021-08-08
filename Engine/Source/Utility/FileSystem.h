#pragma once
#include "Core/GearDefine.h"

#include <string>
namespace gear {
    std::string getCurrentPath();

    std::string readFileData(const std::string& path);
}