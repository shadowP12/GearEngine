#ifndef SHADER_H
#define SHADER_H
#include "../RenderAPI/VulkanContext.h"
class Shader
{
public:
	Shader(const std::vector<uint32_t> &spirv)
	{
		VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = spirv.size() * sizeof(uint32_t);
		moduleCreateInfo.pCode = spirv.data();

		if (vkCreateShaderModule(VulkanContext::instance().getDevice(), &moduleCreateInfo, NULL, &mShaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module!");
		}
	}
	~Shader()
	{
		vkDestroyShaderModule(VulkanContext::instance().getDevice(), mShaderModule, nullptr);
	}
	VkShaderModule getShaderModule() { return mShaderModule; }
private:
	VkShaderModule mShaderModule;
};
#endif