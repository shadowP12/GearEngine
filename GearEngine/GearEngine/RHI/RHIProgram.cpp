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
}
