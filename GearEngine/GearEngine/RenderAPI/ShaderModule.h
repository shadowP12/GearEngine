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
		if (mInfo)
			delete mInfo;
	}
	void load(const char* path);
	VkShaderModule getShaderModule() { return mShaderModule; }
	ProgramInfo* getInfo() { return mInfo; }
private:
	VkShaderModule mShaderModule;
	ProgramInfo* mInfo;
};

#endif