#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H
#include "HardwareBuffer.h"

class IndexBuffer : public HardWareBuffer
{
public:
	IndexBuffer(uint32_t elementSize, uint32_t indexCount)
		: HardWareBuffer(INDEX, GBU_DYNAMIC, elementSize*indexCount)
	{
	}
	~IndexBuffer()
	{
	}

private:

};
#endif
