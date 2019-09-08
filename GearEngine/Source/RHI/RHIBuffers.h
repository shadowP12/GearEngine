#ifndef RHI_BUFFER_H
#define RHI_BUFFER_H
#include "RHIDefine.h"

//只使用vulkan的话就不用单独再抽象一个RHIBufferUsageBit
class RHIDevice;
class RHIBuffer
{
public:
	RHIBuffer(RHIDevice* device, VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
	~RHIBuffer();
	void readData(uint32_t offset, uint32_t size, void* dest);
	void writeData(uint32_t offset, uint32_t size, void* source);
	VkBuffer getBuffer() { return mBuffer; }
	VkDeviceMemory getMemory() { return mMemory; }
	VkDeviceSize getSize() { return mSize; }
private:
	RHIDevice* mDevice;
	VkBuffer mBuffer;
	VkDeviceMemory mMemory;
	VkDeviceSize mSize;
};

class RHIVertexBuffer : public RHIBuffer
{
public:
	RHIVertexBuffer(RHIDevice* device, uint32_t elementSize, uint32_t vertexCount);
	~RHIVertexBuffer();
private:
};

class RHIIndexBuffer : public RHIBuffer
{
public:
	RHIIndexBuffer(RHIDevice* device, uint32_t elementSize, uint32_t indexCount);
	~RHIIndexBuffer();
	uint32_t getIndexCount() { return mIndexCount; }
private:
	uint32_t mIndexCount;
};

class RHIUniformBuffer : public RHIBuffer
{
public:
	RHIUniformBuffer(RHIDevice* device, uint32_t size);
	~RHIUniformBuffer();
private:
};

class RHITransferBuffer : public RHIBuffer
{
public:
	RHITransferBuffer(RHIDevice* device, uint32_t size);
	~RHITransferBuffer();
private:
};

#endif