#include "RHIProgramParam.h"
#include "RHIDevice.h"

RHIProgramParam::RHIProgramParam()
{
}

RHIProgramParam::~RHIProgramParam()
{
}

RHIProgramParamList::RHIProgramParamList(RHIDevice* device, const std::map<uint32_t, VkDescriptorSet>& descriptorSets,
										const RHIParamInfo& vertexProgramParamInfo, const RHIParamInfo& fragmentProgramParamInfo)
	:mDevice(device),
	mDescriptorSets(descriptorSets),
	mVertexProgramParamInfo(vertexProgramParamInfo),
	mFragmentProgramParamInfo(fragmentProgramParamInfo)
{
}

RHIProgramParamList::~RHIProgramParamList()
{
}