#ifndef RHI_QUEUE_H
#define RHI_QUEUE_H
#include "RHIDefine.h"

class RHIDevice;
class RHISwapChain;
class RHICommandBuffer;
class RHIFence;
class RHISemaphore;

struct RHIQueueSubmitInfo
{
    RHICommandBuffer* cmdBuf;
    uint32_t waitSemaphoreCount = 0;
    RHISemaphore** waitSemaphores = nullptr;
    uint32_t signalSemaphoreCount = 0;
    RHISemaphore** signalSemaphores = nullptr;
};

struct RHIQueuePresentInfo
{
    RHISwapChain* swapChain;
    uint32_t waitSemaphoreCount;
    RHISemaphore** waitSemaphores;
    uint32_t index;
};

class RHIQueue
{
public:
	RHIQueue(RHIDevice* device, VkQueue queue, uint32_t familyIndex, QueueType type);
	~RHIQueue();
	VkQueue getHandle() { return mQueue; }
	uint32_t getFamilyIndex() { return mFamilyIndex; }
	QueueType getType() { return mType; }
	void submit(const RHIQueueSubmitInfo& info);
	void Present(const RHIQueuePresentInfo& info);
	void waitIdle();
private:
	RHIDevice* mDevice;
	VkQueue mQueue;
	uint32_t mFamilyIndex;
	QueueType mType;
};

#endif
