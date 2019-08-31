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

class RHIDevice
{
public:
	RHIDevice(VkPhysicalDevice gpu);
	~RHIDevice();
	VkDevice getDevice() { return mDevice; }
	VkPhysicalDevice getPhyDevice() { return mGPU; }
	RHIQueue* getGraphicsQueue() { return mGraphicsQueue; }
	uint32_t findMemoryType(const uint32_t &typeFilter, const VkMemoryPropertyFlags &properties);
	RHIBuffer* createBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
	RHIUniformBuffer* createUniformBuffer(uint32_t size);
	RHIVertexBuffer* createVertexBuffer(uint32_t elementSize, uint32_t vertexCount);
	RHIIndexBuffer* createIndexBuffer(uint32_t elementSize, uint32_t indexCount);
	RHITexture* createTexture(const RHITextureInfo& textureInfo);
	RHITextureView* createTextureView(const RHITextureViewInfo& viewInfo);
	RHITextureView* createTextureView(VkImageView view);
	RHICommandBuffer* allocCommandBuffer(const CommandBufferType& type, bool primary);
	RHIProgram* createProgram(const RHIProgramInfo& programInfo);
	RHIRenderPass* createRenderPass(const RHIRenderPassInfo& renderPassInfo);
	RHIFramebuffer* createFramebuffer(const RHIFramebufferInfo& framebufferInfo);
private:
	void createCommandPool();
private:
	friend class RHI;
	VkPhysicalDevice mGPU;
	VkDevice mDevice;
	VkPhysicalDeviceProperties mDeviceProperties;
	VkPhysicalDeviceFeatures mDeviceFeatures;
	VkPhysicalDeviceMemoryProperties mMemoryProperties;
	RHICommandBufferPool* mGraphicsCommandPool;
	RHICommandBufferPool* mComputeCommandPool;
	RHICommandBufferPool* mTransferCommandPool;
	RHIQueue* mGraphicsQueue;
	RHIQueue* mComputeQueue;
	RHIQueue* mTransferQueue;
};

#endif