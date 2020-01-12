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
class RHIGraphicsPipelineState;
class RHIRenderPass;
class RHIFramebuffer;

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
	// 绘制相关命令
	void beginRenderPass(glm::vec4 renderArea);
	void endRenderPass();
	void bindVertexBuffer(RHIVertexBuffer* vertexBuffer);
	void bindIndexBuffer(RHIIndexBuffer* indexBuffer);
	void bindGraphicsPipelineState(RHIGraphicsPipelineState* pipelineState);
	void setRenderTarget(RHIFramebuffer* framebuffer);
	void setViewport(glm::vec4 viewport);
	void setScissor(glm::vec4 scissor);
	void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
	void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
	bool isInRenderPass() const { return mState == State::RecordingRenderPass; }
	// 传输相关命令
	void setImageLayout();
	void copyBufferToImage();
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
	RHIDevice* mDevice = nullptr;
	RHIQueue* mQueue = nullptr;
	RHICommandBufferPool* mCommandPool = nullptr;
	RHIFramebuffer* mFramebuffer = nullptr;
	RHIGraphicsPipelineState* mGraphicsPipelineState = nullptr;
	RHIVertexBuffer* mVertexBuffer = nullptr;
	RHIIndexBuffer* mIndexBuffer = nullptr;
	glm::vec4 mViewport;
	glm::vec4 mScissor;
	VkCommandBuffer mCommandBuffer;
	VkFence mFence;
	std::vector<VkSemaphore> mWaitSemaphores;
	State mState;
};

#endif
