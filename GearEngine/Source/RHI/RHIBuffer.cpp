#include "RHIBuffer.h"
#include "RHIDevice.h"
#include <vector>

RHIBuffer::RHIBuffer(RHIDevice* device, const RHIBufferInfo& info)
	:mDevice(device)
{
    mSize = info.size;
    mDescriptors = info.descriptors;
    mCurrentState = RESOURCE_STATE_UNDEFINED;//info.initialState;
    mMemoryUsage = info.memoryUsage;

	VkBufferCreateInfo bufferInfo;
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.flags = 0;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.usage = toVkBufferUsage(info.descriptors);
	bufferInfo.queueFamilyIndexCount = 0;
	bufferInfo.pQueueFamilyIndices = nullptr;
	bufferInfo.size = info.size;
    if (info.memoryUsage == RESOURCE_MEMORY_USAGE_GPU_ONLY || info.memoryUsage == RESOURCE_MEMORY_USAGE_GPU_TO_CPU)
        bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	CHECK_VKRESULT(vkCreateBuffer(mDevice->getDevice(), &bufferInfo, nullptr, &mBuffer));

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(mDevice->getDevice(), mBuffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = mDevice->findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    CHECK_VKRESULT(vkAllocateMemory(mDevice->getDevice(), &memoryAllocateInfo, nullptr, &mMemory));

	vkBindBufferMemory(mDevice->getDevice(), mBuffer, mMemory, 0);
}

RHIBuffer::~RHIBuffer()
{
	vkDestroyBuffer(mDevice->getDevice(), mBuffer, nullptr);
	vkFreeMemory(mDevice->getDevice(), mMemory, nullptr);
}

void RHIBuffer::readData(uint32_t offset, uint32_t size, void * dest)
{
	void* data;
	vkMapMemory(mDevice->getDevice(), mMemory, offset, size, 0, &data);
	memcpy(dest, data, static_cast<size_t>(size));
	vkUnmapMemory(mDevice->getDevice(), mMemory);
}

void RHIBuffer::writeData(uint32_t offset, uint32_t size, void * source)
{
	void *data;
	vkMapMemory(mDevice->getDevice(), mMemory, offset, size, 0, &data);
	memcpy(data, source, static_cast<size_t>(size));
	vkUnmapMemory(mDevice->getDevice(), mMemory);
}