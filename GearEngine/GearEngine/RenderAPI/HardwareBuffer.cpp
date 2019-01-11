#include "HardwareBuffer.h"
HardWareBuffer::HardWareBuffer(BufferType type, GpuBufferUsage usage, VkDeviceSize size)
{
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
	}

	mBuffer = VulkanContext::instance().getResourceManager()->create<Buffer>(size, usageFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

HardWareBuffer::~HardWareBuffer()
{
	mBuffer->destroy();
}

void HardWareBuffer::readData(uint32_t offset, uint32_t size, void * dest)
{
	mBuffer->readData(offset,size,dest);
}

void HardWareBuffer::writeData(uint32_t offset, uint32_t size, void * source)
{
	mBuffer->writeData(offset,size,source);
}
