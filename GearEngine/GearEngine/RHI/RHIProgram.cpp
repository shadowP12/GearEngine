#include "RHIProgram.h"
#include "RHIDevice.h"
#include "Managers/RHIProgramManager.h"

RHIProgram::RHIProgram(RHIDevice* device, const RHIProgramInfo& info)
	:mDevice(device), mSource(info.source), mEntryPoint(info.entryPoint), mType(info.type)
{
}

RHIProgram::~RHIProgram()
{
	vkDestroyShaderModule(mDevice->getDevice(), mModule, nullptr);
}

void RHIProgram::compile()
{
	RHIProgramManager::instance().compile(this);
	VkShaderModuleCreateInfo moduleInfo;
	moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo.pNext = nullptr;
	moduleInfo.flags = 0;
	moduleInfo.codeSize = mBytecode.size();
	moduleInfo.pCode = mBytecode.data();

	if (vkCreateShaderModule(mDevice->getDevice(), &moduleInfo, nullptr, &mModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}
}
