#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H
#include "HardwareBuffer.h"

class VertexBuffer : public HardWareBuffer
{
public:
	VertexBuffer(uint32_t elementSize, uint32_t vertexCount)
		: HardWareBuffer(VERTEX, GBU_DYNAMIC, elementSize*vertexCount)
	{
	}
	~VertexBuffer() {}

private:

};
#endif
