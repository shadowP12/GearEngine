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

void RHIProgramParamList::setParamBlockBuffer(std::string name, RHIUniformBuffer* buffer)
{
	uint32_t flag = 0;
	uint32_t set = -1;
	uint32_t binding = -1;
	uint32_t size = 0;

	for (auto& entry : mVertexProgramParamInfo.paramBlocks)
	{
		if (entry.first == name)
		{
			flag = 1;
			set = entry.second.set;
			binding = entry.second.slot;
			size = entry.second.blockSize;
			break;
		}
	}
	for (auto& entry : mFragmentProgramParamInfo.paramBlocks)
	{
		if (entry.first == name)
		{
			flag = 1;
			set = entry.second.set;
			binding = entry.second.slot;
			size = entry.second.blockSize;
			break;
		}
	}
	if (flag == 0)
		return;
	
	
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = buffer->getBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = size;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = set;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(mDevice->getDevice(), 1, &descriptorWrite, 0, nullptr);
}