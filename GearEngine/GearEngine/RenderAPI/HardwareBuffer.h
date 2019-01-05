#ifndef HARDWARE_BUFFER_H
#define HARDWARE_BUFFER_H
#include "Buffer.h"

class HardWareBuffer
{
public:
	enum BufferType
	{
		VERTEX,
		INDEX,
		UNIFORM
	};
	HardWareBuffer(BufferType type, GpuBufferUsage usage, uint32_t size);
	~HardWareBuffer();
	void readData(uint32_t offset, uint32_t size, void* dest);
	void writeData(uint32_t offset, uint32_t size, void* source);
private:
	void createBuffer(uint32_t size);
private:
	Buffer* mBuffer;
	Buffer* mStagingBuffer;
	VkBufferCreateInfo mBufferCI;
	VkBufferUsageFlags mUsageFlags;
	bool mDirectlyMappable;
};


#endif