#ifndef RHI_BUFFER_H
#define RHI_BUFFER_H
#include "RHIDefine.h"

class RHIDevice;

struct RHIBufferInfo {
    DescriptorType descriptors;
    //ResourceState initialState;
    ResourceMemoryUsage memoryUsage;
    uint32_t size;
};

class RHIBuffer
{
public:
	RHIBuffer(RHIDevice* device, const RHIBufferInfo& info);
	~RHIBuffer();
	void readData(uint32_t offset, uint32_t size, void* dest);
	void writeData(uint32_t offset, uint32_t size, void* source);
	VkBuffer getHandle() { return mBuffer; }
	VkDeviceMemory getMemory() { return mMemory; }
	uint32_t getSize() { return mSize; }
	DescriptorType getDescriptorType() { return mDescriptors; }
    ResourceState getResourceState() { return mCurrentState; }
    void setResourceState(ResourceState state) { mCurrentState = state; }
private:
	RHIDevice* mDevice;
	VkBuffer mBuffer;
	VkDeviceMemory mMemory;
	uint32_t mSize;
    DescriptorType mDescriptors;
    ResourceState mCurrentState;
    ResourceMemoryUsage mMemoryUsage;
};

#endif