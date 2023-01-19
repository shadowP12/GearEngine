#pragma once
#include "Core/GearDefine.h"

#include <filesystem/path.h>
#include <filesystem/resolver.h>

#include <string>

namespace gear {
    int SaveFile(const std::string& path, void* data, uint32_t size);

    int LoadFile(const std::string& path, void** data, uint32_t& size);
}