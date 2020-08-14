#include "RHIProgram.h"
#include "RHIDevice.h"
#include "RHIBuffer.h"
#include "RHITexture.h"

RHIProgram::RHIProgram(RHIDevice* device, const RHIProgramInfo& info)
	:mDevice(device), mType(info.type)
{
    VkShaderModuleCreateInfo moduleInfo;
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleInfo.pNext = nullptr;
    moduleInfo.flags = 0;
    moduleInfo.codeSize = info.bytes.size() * sizeof(uint32_t);
    moduleInfo.pCode = info.bytes.data();

    CHECK_VKRESULT(vkCreateShaderModule(mDevice->getDevice(), &moduleInfo, nullptr, &mModule));
}

RHIProgram::~RHIProgram()
{
	vkDestroyShaderModule(mDevice->getDevice(), mModule, nullptr);
}