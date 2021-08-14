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

        Material* Compile(const std::string& code);

    private:
        static void ProcessShadingModel(Material::Builder&, const std::string&);

        static void ProcessBlendingModel(Material::Builder&, const std::string&);

    private:
        using Callback = void(*)(Material::Builder*, const std::string&);
        std::unordered_map<std::string, Callback> mParameters;
    };
}