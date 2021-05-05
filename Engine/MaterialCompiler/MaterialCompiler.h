#pragma once
#include "Resource/Material.h"
#include <Blast/Gfx/GfxDefine.h>
#include <string>

namespace gear {
    struct MaterialVariantInfo {
        Blast::ShaderStage stage;
        uint8_t variant;
    };

    struct MaterialBuildInfo {
        std::vector<std::pair<std::string, std::string>> uniforms;
        std::vector<std::pair<std::string, std::string>> samplers;
    };

    class MaterialCompiler {
    public:
        MaterialCompiler();
        ~MaterialCompiler();
        Material* compile(const std::string& path);
    private:

    private:

    };
}