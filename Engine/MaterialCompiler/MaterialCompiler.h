#pragma once
#include "Resource/Material.h"
#include <Blast/Gfx/GfxDefine.h>
#include <string>

namespace Blast {
    class ShaderCompiler;
}

namespace gear {
    struct MaterialVariantInfo {
        MaterialVariantInfo(uint8_t v, Blast::ShaderStage s) : variant(v), stage(s) {}
        uint8_t variant;
        Blast::ShaderStage stage;
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
        Blast::ShaderCompiler* mShaderCompiler = nullptr;
    };
}