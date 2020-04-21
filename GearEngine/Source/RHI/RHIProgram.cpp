#include "RHIProgram.h"
#include "RHIDevice.h"
#include "Managers/RHIProgramManager.h"

RHIProgram::RHIProgram(RHIDevice* device, RHIProgramManager* mgr, const RHIProgramInfo& info)
	:mDevice(device), mProgramMgr(mgr), mSource(info.source), mEntryPoint(info.entryPoint), mType(info.type)
{
    mIsCompiled = false;
}

RHIProgram::~RHIProgram()
{
    mProgramMgr->deleteProgram(this);
	vkDestroyShaderModule(mDevice->getDevice(), mModule, nullptr);
}

void RHIProgram::compile()
{
	mProgramMgr->compile(this);
	VkShaderModuleCreateInfo moduleInfo;
	moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo.pNext = nullptr;
	moduleInfo.flags = 0;
	moduleInfo.codeSize = mBytecode.size() * sizeof(uint32_t);
	moduleInfo.pCode = mBytecode.data();

	if (vkCreateShaderModule(mDevice->getDevice(), &moduleInfo, nullptr, &mModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

    mIsCompiled = true;

	// 创建
}
