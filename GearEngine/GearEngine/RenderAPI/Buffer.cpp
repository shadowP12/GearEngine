#include "Buffer.h"

Buffer::Buffer(VulkanResourceManager* manager, VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags)
	:VulkanResource(manager), mSize(size)
{
	VkBufferCreateInfo bufferCI;
	bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCI.pNext = nullptr;
	bufferCI.flags = 0;
	bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCI.usage = usageFlags;
	bufferCI.queueFamilyIndexCount = 0;
	bufferCI.pQueueFamilyIndices = nullptr;
	bufferCI.size = size;

	if (vkCreateBuffer(VulkanContext::instance().getDevice(), &bufferCI, nullptr, &mBuffer))
	{
		throw std::runtime_error("failed to create buffer!");
	}

	// Allocates buffer memory.
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(VulkanContext::instance().getDevice(), mBuffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = VulkanContext::instance().findMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	if (vkAllocateMemory(VulkanContext::instance().getDevice(), &memoryAllocateInfo, nullptr, &mMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(VulkanContext::instance().getDevice(), mBuffer, mMemory, 0);
}

Buffer::~Buffer()
{
	vkDestroyBuffer(VulkanContext::instance().getDevice(), mBuffer, nullptr);
	vkFreeMemory(VulkanContext::instance().getDevice(), mMemory, nullptr);
}

void Buffer::readData(uint32_t offset, uint32_t size, void * dest)
{
	void* data;
	vkMapMemory(VulkanContext::instance().getDevice(), mMemory, offset, size, 0, &data);
	memcpy(dest, data, static_cast<size_t>(size));
	vkUnmapMemory(VulkanContext::instance().getDevice(), mMemory);
}

void Buffer::writeData(uint32_t offset, uint32_t size, void * source)
{
	void *data;
	vkMapMemory(VulkanContext::instance().getDevice(), mMemory, offset, size, 0, &data);
	memcpy(data, source, static_cast<size_t>(size));
	vkUnmapMemory(VulkanContext::instance().getDevice(), mMemory);
}