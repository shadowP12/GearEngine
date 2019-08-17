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
	HardWareBuffer(BufferType type, GpuBufferUsage usage, VkDeviceSize size);
	~HardWareBuffer();
	void readData(uint32_t offset, uint32_t size, void* dest);
	void writeData(uint32_t offset, uint32_t size, void* source);
	Buffer* getBuffer() { return mBuffer; }

private:
	Buffer* mBuffer;
};


#endif