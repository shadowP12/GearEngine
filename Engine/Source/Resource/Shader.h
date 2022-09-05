#pragma once
#include "Core/GearDefine.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace blast
{
	enum ShaderStage;
    class GfxShader;
}

namespace gear
{
    struct SimpleShaderDesc
    {
		blast::ShaderStage stage;
        std::string code;
        std::vector<std::string> defines;
    };

    class SimpleShader
    {
    public:
        SimpleShader(const SimpleShaderDesc& desc);

        ~SimpleShader();

        blast::GfxShader* GetShader(uint32_t variant = 0);

    protected:
		blast::ShaderStage stage;
        std::string code;
        std::vector<std::string> defines;
		std::unordered_map<uint32_t, blast::GfxShader*> cache_shaders;
    };
}