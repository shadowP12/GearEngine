#include "Buffer.h"

Buffer::Buffer(VulkanResourceManager* manager, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize size)
	:VulkanResource(manager), mBuffer(buffer), mBufferMemory(memory), mSize(size)
{
}

Buffer::~Buffer()
{

}