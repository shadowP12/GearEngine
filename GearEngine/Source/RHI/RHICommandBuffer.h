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
class RHIBuffer;
class RHITexture;
class RHIGraphicsPipeline;
class RHIComputePipeline;
class RHIProgram;
class RHIRenderPass;
class RHIFramebuffer;
class RHIFence;
class RHISemaphore;
class RHIDescriptorSet;
class RHIBufferBarrier;
class RHITextureBarrier;

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

struct SubresourceDataInfo
{
    uint32_t mipLevel;
    uint32_t arrayLayer;
};

class RHICommandBuffer
{
public:
	RHICommandBuffer(RHIDevice* device, RHIQueue* queue, RHICommandBufferPool* pool);
	~RHICommandBuffer();
	VkCommandBuffer getHandle() { return mCommandBuffer; }
	void begin();
	void end();
    void bindFramebuffer(RHIFramebuffer* fb);
    void unbindFramebuffer();
	void setViewport(int x, int y, int w, int h);
    void setScissor(int x, int y, int w, int h);
	void bindVertexBuffer(RHIBuffer* vertexBuffer, uint32_t offset);
	void bindIndexBuffer(RHIBuffer* indexBuffer, uint32_t offset, VkIndexType type);
    void bindGraphicsPipeline(RHIGraphicsPipeline* pipeline, RHIDescriptorSet** descriptorSets, uint32_t count);
    void bindComputePipeline(RHIComputePipeline* pipeline, RHIDescriptorSet** descriptorSets, uint32_t count);
	void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
	void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
    void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
    void setResourceBarrier(uint32_t bufferBarrierCount, RHIBufferBarrier* bufferBarriers, uint32_t textureBarrierCount, RHITextureBarrier* textureBarriers);
    void updateBuffer(RHIBuffer* dstBuffer, uint32_t dstOffset, RHIBuffer* srcBuffer, uint32_t srcOffset, uint32_t size);
    void updateSubresource(RHITexture* dstTexture, RHIBuffer* srcBuffer, const SubresourceDataInfo& info);
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
	friend class RHIQueue;
	RHIDevice* mDevice = nullptr;
	RHIQueue* mQueue = nullptr;
	RHICommandBufferPool* mCommandPool = nullptr;
	RHIFence* mFence = nullptr;
	VkCommandBuffer mCommandBuffer;
	State mState;
};

#endif
