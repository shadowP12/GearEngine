#include "ShaderModule.h"

void ShaderModule::load(const char * path)
{
	std::vector<uint32_t> spirv_binary = read_spirv_file(path);

	mInfo = std::shared_ptr<ProgramInfo>(getProgramInfo(spirv_binary));

	VkShaderModuleCreateInfo moduleCreateInfo{};
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.codeSize = spirv_binary.size();
	moduleCreateInfo.pCode = spirv_binary.data();

	if (vkCreateShaderModule(VulkanContext::instance().getDevice(), &moduleCreateInfo, NULL, &mShaderModule) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create shader module!");
	}
}
