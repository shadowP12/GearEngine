#pragma once
#include "Core/GearDefine.h"

#include <filesystem/path.h>
#include <filesystem/resolver.h>

#include <string>

namespace gear {
    std::string GetCurrentPath();

    std::string ReadFileData(const std::string& path);
}