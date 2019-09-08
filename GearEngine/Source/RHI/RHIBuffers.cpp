#include "RHIBuffers.h"
#include "RHIDevice.h"
/**
  usageFlags : bufferÓÃÍ¾
  memoryPropertyFlags : buffer·¶Î§
*/
RHIBuffer::RHIBuffer(RHIDevice* device, VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags)
	:mDevice(device), mSize(size)
{
	VkBufferCreateInfo bufferInfo;
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.flags = 0;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.usage = usageFlags;
	bufferInfo.queueFamilyIndexCount = 0;
	bufferInfo.pQueueFamilyIndices = nullptr;
	bufferInfo.size = size;

	if (vkCreateBuffer(mDevice->getDevice(), &bufferInfo, nullptr, &mBuffer))
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(mDevice->getDevice(), mBuffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = mDevice->findMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	if (vkAllocateMemory(mDevice->getDevice(), &memoryAllocateInfo, nullptr, &mMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

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

RHIVertexBuffer::RHIVertexBuffer(RHIDevice* device, uint32_t elementSize, uint32_t vertexCount)
	: RHIBuffer(device, elementSize * vertexCount, 
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
{
}

RHIVertexBuffer::~RHIVertexBuffer() 
{
}

RHIIndexBuffer::RHIIndexBuffer(RHIDevice* device, uint32_t elementSize, uint32_t indexCount)
	: RHIBuffer(device, elementSize*indexCount,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
{
	mIndexCount = indexCount;
}

RHIIndexBuffer::~RHIIndexBuffer()
{
}

RHIUniformBuffer::RHIUniformBuffer(RHIDevice* device, uint32_t size)
	: RHIBuffer(device, size,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
{
}
RHIUniformBuffer::~RHIUniformBuffer()
{
}

RHITransferBuffer::RHITransferBuffer(RHIDevice* device, uint32_t size)
	:RHIBuffer(device, size,
			   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
{
}
RHITransferBuffer::~RHITransferBuffer()
{
}