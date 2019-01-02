#ifndef BUFFER_H
#define BUFFER_H
#include "VulkanContext.h"
#include "VulkanResource.h"

class Buffer : public VulkanResource
{
public:
	Buffer(VulkanResourceManager* manager, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize size);
	~Buffer();
	VkBuffer getBuffer() { return mBuffer; }
	VkDeviceMemory getMemory() { return mBufferMemory; }
	VkDeviceSize getSize() { return mSize; }
private:
	VkBuffer mBuffer;
	VkDeviceMemory mBufferMemory;
	VkDeviceSize mSize;
};
#endif