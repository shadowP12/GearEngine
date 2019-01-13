#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H
#include "HardwareBuffer.h"

class IndexBuffer : public HardWareBuffer
{
public:
	IndexBuffer(uint32_t elementSize, uint32_t indexCount)
		: HardWareBuffer(INDEX, GBU_DYNAMIC, elementSize*indexCount), mIndexCount(static_cast<uint32_t>(indexCount))
	{
	}
	~IndexBuffer()
	{
	}
	uint32_t getIndexCount() { return mIndexCount; }

private:
	uint32_t mIndexCount;
};
#endif
