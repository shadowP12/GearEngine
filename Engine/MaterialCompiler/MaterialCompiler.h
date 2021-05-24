#pragma once
#include "Resource/Material.h"
#include <Blast/Gfx/GfxDefine.h>
#include <string>

namespace Blast {
    class ShaderCompiler;
}

namespace gear {
    struct MaterialBuildInfo {
        Blast::ShaderSemantic semantics = Blast::SEMANTIC_UNDEFINED;
        std::vector<std::pair<std::string, std::string>> uniforms;
        std::vector<std::pair<std::string, std::string>> samplers;
    };

    class MaterialCompiler {
    public:
        MaterialCompiler();

        ~MaterialCompiler();

        Material* compile(const std::string& path);

    private:
        static void processShading(Material* mat, const std::string& value);

    private:
        Blast::ShaderCompiler* mShaderCompiler = nullptr;
        using Callback = void(*)(Material*, const std::string&);
        std::unordered_map<std::string, Callback> mParameters;
    };
}