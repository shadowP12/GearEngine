#ifndef RHI_DEVICE_H
#define RHI_DEVICE_H
#include "RHIDefine.h"
#include "RHIBuffers.h"
#include "RHICommandBuffer.h"
#include "RHIQueue.h"
#include "RHIProgram.h"
#include "RHIRenderPass.h"
#include "RHIFramebuffer.h"
#include "RHITexture.h"
#include "RHITextureView.h"
#include <vector>

class RHIContext;
class RHIProgramManager;
class RHIFence;
class RHISemaphore;

class RHIDevice
{
public:
	RHIDevice(VkPhysicalDevice gpu);
	~RHIDevice();
	VkDevice getDevice() { return mDevice; }
	VkPhysicalDevice getPhyDevice() { return mGPU; }
    VkDescriptorPool getDescriptorPool(){return mDescriptorPool;}
	RHIQueue* getGraphicsQueue() { return mGraphicsQueue; }
	uint32_t findMemoryType(const uint32_t &typeFilter, const VkMemoryPropertyFlags &properties);
	RHIBuffer* createBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
	RHIUniformBuffer* createUniformBuffer(uint32_t size);
	RHIVertexBuffer* createVertexBuffer(uint32_t elementSize, uint32_t vertexCount);
	RHIIndexBuffer* createIndexBuffer(uint32_t elementSize, uint32_t indexCount);
	RHITransferBuffer* createTransferBuffer(uint32_t size);
	RHITexture* createTexture(const RHITextureInfo& textureInfo);
	RHITextureView* createTextureView(const RHITextureViewInfo& viewInfo);
	RHITextureView* createTextureView(VkImageView view);
	RHIProgram* createProgram(const RHIProgramInfo& programInfo);
	RHIRenderPass* createRenderPass(const RHIRenderPassInfo& renderPassInfo);
	RHIFramebuffer* createFramebuffer(const RHIFramebufferInfo& framebufferInfo);
	RHIUniformBuffer* getDummyUniformBuffer() { return mDummyUniformBuffer; };
    RHIFence* createFence();
    RHISemaphore* createSemaphore();
    RHICommandBufferPool* getHelperCmdBufferPool();
    RHIContext* createContext();
    RHICommandBufferPool* createCmdBufferPool();
private:
	friend class RHI;
	friend class RHIContext;
	VkPhysicalDevice mGPU;
	VkDevice mDevice;
	VkPhysicalDeviceProperties mDeviceProperties;
	VkPhysicalDeviceFeatures mDeviceFeatures;
	VkPhysicalDeviceMemoryProperties mMemoryProperties;
    VkDescriptorPool mDescriptorPool;
    RHIProgramManager* mProgramMgr;
    RHICommandBufferPool* mHelperCommandPool;
	RHIQueue* mGraphicsQueue;
	RHIQueue* mComputeQueue;
	RHIQueue* mTransferQueue;
	//
	RHIUniformBuffer* mDummyUniformBuffer;
};

#endif