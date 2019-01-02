#include "HardwareBuffer.h"
HardWareBuffer::HardWareBuffer(BufferType type, GpuBufferUsage usage, uint32_t size)
{
	mDirectlyMappable = (usage & GBU_DYNAMIC) != 0;
	VkBufferUsageFlags usageFlags = 0;
	switch (type)
	{
	case BufferType::VERTEX:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		if ((usage & GBU_LOADSTORE) == GBU_LOADSTORE)
			usageFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		break;
	case INDEX:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		if ((usage & GBU_LOADSTORE) == GBU_LOADSTORE)
			usageFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		break;
	case UNIFORM:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		break;
	case GENERIC:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

		if ((usage & GBU_LOADSTORE) == GBU_LOADSTORE)
			usageFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

		break;
	case STRUCTURED:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		break;
	}

	mBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	mBufferCI.pNext = nullptr;
	mBufferCI.flags = 0;
	mBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	mBufferCI.usage = usageFlags;
	mBufferCI.queueFamilyIndexCount = 0;
	mBufferCI.pQueueFamilyIndices = nullptr;

	createBuffer(size);
}

HardWareBuffer::~HardWareBuffer()
{
	mBuffer->destroy();
}

void HardWareBuffer::readData(uint32_t offset, uint32_t size, void * dest)
{
	void* data;
	vkMapMemory(VulkanContext::instance().getDevice(), mBuffer->getMemory(), offset, size, 0, &data);
	memcpy(dest, data, static_cast<size_t>(size));
	vkUnmapMemory(VulkanContext::instance().getDevice(), mBuffer->getMemory());
}

void HardWareBuffer::writeData(uint32_t offset, uint32_t size, void * source)
{
	void *data;
	vkMapMemory(VulkanContext::instance().getDevice(), mBuffer->getMemory(), offset, size, 0, &data);
	memcpy(data, source, static_cast<size_t>(size));
	vkUnmapMemory(VulkanContext::instance().getDevice(), mBuffer->getMemory());
}

void HardWareBuffer::createBuffer(uint32_t size)
{
	VkBufferUsageFlags usage = mBufferCI.usage;

	mBufferCI.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	mBufferCI.size = size;

	VkMemoryPropertyFlags flags = mDirectlyMappable ?
		(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) :
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	VkBuffer buffer;
	if (vkCreateBuffer(VulkanContext::instance().getDevice(), &mBufferCI, nullptr, &buffer))
	{
		throw std::runtime_error("failed to create buffer!");
	}

	// Allocates buffer memory.
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(VulkanContext::instance().getDevice(), buffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = VulkanContext::instance().findMemoryType(memoryRequirements.memoryTypeBits, flags);

	VkDeviceMemory memory;
	if (vkAllocateMemory(VulkanContext::instance().getDevice(), &memoryAllocateInfo, nullptr, &memory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(VulkanContext::instance().getDevice(), buffer, memory, 0);

	mBuffer = VulkanContext::instance().getResourceManager()->create<Buffer>(buffer, memory, size);
	mBufferCI.usage = usage;
}
