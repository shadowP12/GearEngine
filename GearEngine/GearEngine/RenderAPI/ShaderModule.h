#ifndef SHADER_MODULE_H
#define SHADER_MODULE_H
#include "VulkanContext.h"
#include "../Texture/Texture.h"
#include "../Utility/ShaderTool/ShaderTool.h"

class ShaderModule
{
public:
	ShaderModule()
	{
	}
	~ShaderModule()
	{
		vkDestroyShaderModule(VulkanContext::instance().getDevice(),mShaderModule,nullptr);
	}
	void load(const char* path);
	VkShaderModule getShaderModule() { return mShaderModule; }
	std::shared_ptr<ProgramInfo> getInfo() { return mInfo; }
private:
	VkShaderModule mShaderModule;
	std::shared_ptr<ProgramInfo> mInfo;
};

#endif