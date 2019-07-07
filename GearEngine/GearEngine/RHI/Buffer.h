#ifndef BUFFER_H
#define BUFFER_H
#include "VulkanContext.h"
#include "VulkanResource.h"

class Buffer : public VulkanResource
{
public:
	Buffer(VulkanResourceManager* manager, VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
	~Buffer();
	void readData(uint32_t offset, uint32_t size, void* dest);
	void writeData(uint32_t offset, uint32_t size, void* source);
	VkBuffer getBuffer() { return mBuffer; }
	VkDeviceMemory getMemory() { return mMemory; }
	VkDeviceSize getSize() { return mSize; }
private:
	VkBuffer mBuffer;
	VkDeviceMemory mMemory;
	VkDeviceSize mSize;
};
#endif