#ifndef RHI_COMMANDBUFFER_H
#define RHI_COMMANDBUFFER_H
#include "RHIDefine.h"

enum class CommandBufferType
{
	Graphics,
	Compute,
	Transfer
};

enum class CommandBufferLevel
{
	Primary,
	Secondary
};

class RHIDevice;

class RHICommandBuffer
{
public:
	RHICommandBuffer(RHIDevice* device);
	~RHICommandBuffer();
	VkCommandBuffer getHandle() { return mCommandBuffer; }
	void begin();
	void end();
private:
	friend class RHIDevice;
	RHIDevice* mDevice;
	VkCommandPool mCommandPool;
	VkCommandBuffer mCommandBuffer;
	bool mRunning;
};
#endif