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
	// 为每一个desc属性初始化,缺少这步直接调用set对象会出现报错
	for (auto& e : mDescriptorSets)
	{
		uint32_t set = e.first;
		VkDescriptorSet descSet = e.second;
		for (auto& entry : vertexProgramParamInfo.paramBlocks)
		{
			if (entry.second.set == set)
			{
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = mDevice->getDummyUniformBuffer()->getBuffer();
				bufferInfo.offset = 0;
				bufferInfo.range = mDevice->getDummyUniformBuffer()->getSize();

				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = descSet;
				descriptorWrite.dstBinding = entry.second.slot;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &bufferInfo;

				vkUpdateDescriptorSets(mDevice->getDevice(), 1, &descriptorWrite, 0, nullptr);
			}
		}

		for (auto& entry : vertexProgramParamInfo.samplers)
		{
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.sampler = VK_NULL_HANDLE;
			imageInfo.imageView = VK_NULL_HANDLE;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descSet;
			descriptorWrite.dstBinding = entry.second.slot;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(mDevice->getDevice(), 1, &descriptorWrite, 0, nullptr);
		}

		for (auto& entry : fragmentProgramParamInfo.paramBlocks)
		{
			if (entry.second.set == set)
			{
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = mDevice->getDummyUniformBuffer()->getBuffer();
				bufferInfo.offset = 0;
				bufferInfo.range = mDevice->getDummyUniformBuffer()->getSize();

				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = descSet;
				descriptorWrite.dstBinding = entry.second.slot;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &bufferInfo;

				vkUpdateDescriptorSets(mDevice->getDevice(), 1, &descriptorWrite, 0, nullptr);
			}
		}
		for (auto& entry : fragmentProgramParamInfo.samplers)
		{
			if (entry.second.set == set)
			{
				VkDescriptorImageInfo imageInfo = {};
				imageInfo.sampler = VK_NULL_HANDLE;
				imageInfo.imageView = VK_NULL_HANDLE;
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = descSet;
				descriptorWrite.dstBinding = entry.second.slot;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pImageInfo = &imageInfo;

				vkUpdateDescriptorSets(mDevice->getDevice(), 1, &descriptorWrite, 0, nullptr);
			}
		}
	}
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