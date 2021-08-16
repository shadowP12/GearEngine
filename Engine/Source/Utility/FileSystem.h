#pragma once
#include "Core/GearDefine.h"

#include <string>
namespace gear {
    std::string GetCurrentPath();

    std::string ReadFileData(const std::string& path);
}