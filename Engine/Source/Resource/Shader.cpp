#include "Shader.h"
#include "GearEngine.h"

#include <Blast/Gfx/GfxDefine.h>
#include <Blast/Gfx/GfxDevice.h>
#include <Blast/Utility/ShaderCompiler.h>

#include <bitset>

namespace gear
{
    SimpleShader::SimpleShader(const SimpleShaderDesc& desc)
    {
		stage = desc.stage;
        code = desc.code;
        defines = desc.defines;
    }

    SimpleShader::~SimpleShader()
    {
		blast::GfxDevice* device = gEngine.GetDevice();
		for (auto& s : cache_shaders)
		{
			device->DestroyShader(s.second);
		}
    }

	blast::GfxShader* SimpleShader::GetShader(uint32_t variant)
	{
		auto shader_iter = cache_shaders.find(variant);
		if (shader_iter != cache_shaders.end())
		{
			return shader_iter->second;
		}

		std::string define_block;
		std::bitset<32> b(variant);
		for (uint32_t i = 0; i < 32; i++)
		{
			if (b.test(i) && i < defines.size())
			{
				define_block += defines[i] + "\n";
			}
		}

		std::string shader_code = define_block + code;
		blast::ShaderCompileDesc compile_desc;
		compile_desc.code = shader_code;
		compile_desc.stage = stage;
		blast::ShaderCompileResult compile_result = gEngine.GetShaderCompiler()->Compile(compile_desc);

		blast::GfxShaderDesc shader_desc;
		shader_desc.stage = stage;
		shader_desc.bytecode = compile_result.bytes.data();
		shader_desc.bytecode_length = compile_result.bytes.size() * sizeof(uint32_t);
		cache_shaders[variant] = gEngine.GetDevice()->CreateShader(shader_desc);
	}

}