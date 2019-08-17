#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H
#include "HardwareBuffer.h"

class UniformBuffer : public HardWareBuffer
{
public:
	UniformBuffer(const VkDeviceSize &size)
		: HardWareBuffer(UNIFORM, GBU_DYNAMIC,size)
	{
	}
	~UniformBuffer() 
	{
	}

private:

};
#endif
