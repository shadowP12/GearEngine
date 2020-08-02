#ifndef RHI_COMMANDBUFFER_H
#define RHI_COMMANDBUFFER_H
#include "RHIDefine.h"
#include "Math/GMath.h"
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
class RHITransferBuffer;
class RHIPipelineState;
class RHIProgram;
class RHIRenderPass;
class RHIFramebuffer;
class RHIFence;
class RHISemaphore;

class RHICommandBufferPool
{
public:
	RHICommandBufferPool(RHIDevice* device, RHIQueue* queue, bool reset);
	~RHICommandBufferPool();
	RHICommandBuffer* getActiveCmdBuffer();
    VkCommandPool getHandle() { return mPool; }
private:
	friend class RHICommandBuffer;
	RHIDevice* mDevice;
	RHIQueue* mQueue;
	std::vector<RHICommandBuffer*> mCmdBuffers;
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
	void beginRenderPass(glm::vec4 renderArea);
	void endRenderPass();
	void bindVertexBuffer(RHIVertexBuffer* vertexBuffer);
	void bindIndexBuffer(RHIIndexBuffer* indexBuffer);
	void bindVertexProgram(RHIProgram* program);
    void bindFragmentProgram(RHIProgram* program);
	void bindPipelineState();
	void setRenderTarget(RHIFramebuffer* framebuffer);
	void setViewport(glm::vec4 viewport);
	void setScissor(glm::vec4 scissor);
	void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
	void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
	void addWaitSemaphore(RHISemaphore* semaphore);
	void addSignalSemaphore(RHISemaphore* semaphore);
    void submit();
	// ÃüÁî×´Ì¬
	enum class State
	{
		Ready,
		Recording,
		RecordingDone,
		Submitted
	};
    void refreshFenceStatus();
private:
	friend class RHIDevice;
	friend class RHICommandBufferPool;
	RHIDevice* mDevice = nullptr;
	RHIQueue* mQueue = nullptr;
	RHICommandBufferPool* mCommandPool = nullptr;
	RHIFence* mFence = nullptr;
	RHIFramebuffer* mFramebuffer = nullptr;
	RHIPipelineState* mPipelineState = nullptr;
	RHIVertexBuffer* mVertexBuffer = nullptr;
	RHIIndexBuffer* mIndexBuffer = nullptr;
	RHIProgram* mVertexProgram = nullptr;
	RHIProgram* mFragmentProgram = nullptr;
	glm::vec4 mViewport;
	glm::vec4 mScissor;
	VkCommandBuffer mCommandBuffer;
	std::vector<RHISemaphore*> mWaitSemaphores;
    std::vector<RHISemaphore*> mSignalSemaphores;
	State mState;
};

#endif
