#pragma once
#include "Core/GearDefine.h"
#include <string>

namespace gear {
    namespace PlatformMisc {
        bool OpenFileDialog(const std::string& filter, std::string& file);

        bool OpenDirectoryDialog(const std::string dialog_title, const std::string& default_dir, std::string& dir);
    }
}