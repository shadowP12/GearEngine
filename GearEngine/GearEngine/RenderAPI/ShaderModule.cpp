#include "ShaderModule.h"
#include <fstream>

void ShaderModule::load(const char * path)
{
	std::vector<uint32_t> spirv_binary = read_spirv_file(path);

	mInfo = std::shared_ptr<ProgramInfo>(getProgramInfo(spirv_binary));

	std::ifstream is(path, std::ios::binary | std::ios::in | std::ios::ate);
	if (is.is_open()) 
	{
		size_t size = is.tellg();
		is.seekg(0, std::ios::beg);
		char* shaderCode = new char[size];
		is.read(shaderCode, size);
		is.close();
		assert(size > 0);

		VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = size;
		moduleCreateInfo.pCode = (uint32_t*)shaderCode;

		if (vkCreateShaderModule(VulkanContext::instance().getDevice(), &moduleCreateInfo, NULL, &mShaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module!");
		}

		delete[] shaderCode;
	}
	else 
	{
		std::cerr << "Error: Could not open shader file \"" << path << "\"" << std::endl;
		return ;
	}
}
