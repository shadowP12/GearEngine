#pragma once
#include "Resource/Material.h"
#include <Blast/Gfx/GfxDefine.h>
#include <string>

namespace gear {
    class MaterialCompiler {
    public:
        MaterialCompiler();

        ~MaterialCompiler();

        Material* Compile(const std::string& str, bool is_file);

    private:
        static void ProcessShadingModel(Material::Builder&, const std::string&);

        static void ProcessBlendingModel(Material::Builder&, const std::string&);

    private:
        using Callback = void(*)(Material::Builder*, const std::string&);
        std::unordered_map<std::string, Callback> parameters;
    };
}