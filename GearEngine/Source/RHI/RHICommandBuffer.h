#ifndef RHI_COMMANDBUFFER_H
#define RHI_COMMANDBUFFER_H
#include "RHIDefine.h"
#include <vector>
enum class CommandBufferType
{
	GRAPHICS,
	COMPUTE,
	TRANSFER
};

class RHIDevice;
class RHIQueue;
class RHICommandBuffer;
class RHIVertexBuffer;
class RHIIndexBuffer;
class RHIGraphicsPipelineState;
class RHIRenderPass;

class RHICommandBufferPool
{
public:
	RHICommandBufferPool(RHIDevice* device, RHIQueue* queue, bool reset);
	~RHICommandBufferPool();
	RHICommandBuffer* allocCommandBuffer(bool primary);
	void freeCommandBuffer(RHICommandBuffer* cmd);
private:
	friend class RHICommandBuffer;
	RHIDevice* mDevice;
	RHIQueue* mQueue;
	VkCommandPool mPool;
};

class RHICommandBuffer
{
public:
	RHICommandBuffer(RHIDevice* device, RHIQueue* queue, RHICommandBufferPool* pool);
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
	friend class RHICommandBufferPool;
	RHIDevice* mDevice;
	RHIQueue* mQueue;
	RHICommandBufferPool* mCommandPool;
	VkCommandBuffer mCommandBuffer;
	VkFence mFence;
	std::vector<VkSemaphore> mWaitSemaphores;
	State mState;
};
#endif