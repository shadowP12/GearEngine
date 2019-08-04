#ifndef RHI_COMMANDBUFFER_H
#define RHI_COMMANDBUFFER_H
#include "RHIDefine.h"

enum class CommandBufferType
{
	GRAPHICS,
	COMPUTE,
	TRANSFER
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
	enum class State
	{
		Ready,
		Recording,
		RecordingRenderPass,
		RecordingDone,
		Submitted
	};
private:
	friend class RHIDevice;
	RHIDevice* mDevice;
	VkCommandPool mCommandPool;
	VkCommandBuffer mCommandBuffer;
	VkFence mFence;
	std::vector<VkSemaphore> mWaitSemaphores;
	State mState;
};
#endif