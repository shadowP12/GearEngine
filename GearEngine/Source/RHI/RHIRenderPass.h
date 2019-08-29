#ifndef RHI_RENDER_PASS_H
#define RHI_RENDER_PASS_H
#include "RHIDefine.h"
#include "Utility/Hash.h"
#include <unordered_map>
/**
  1.使用hash为索引去缓存vkrenderpass对象(已废弃)
  2.向外暴露参数和结构尽量不使用vulkan原生结构(由于快速实现暂时使用vulkan原生结构)
  3.暂不支持多个subpass
*/

class RHIDevice;

struct RHIColorAttachmentInfo {
	VkFormat format = VK_FORMAT_UNDEFINED;
	uint32_t numSample = 1;
	VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	VkImageLayout initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkImageLayout finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
};

struct RHIDepthStencilAttachmentInfo {
	VkFormat format = VK_FORMAT_UNDEFINED;
	uint32_t numSample = 1;
	VkAttachmentLoadOp depthLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	VkAttachmentStoreOp depthStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	VkImageLayout initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	VkImageLayout finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
};

struct RHIRenderPassInfo {
	RHIColorAttachmentInfo color[8];
	RHIDepthStencilAttachmentInfo depthStencil;
	uint32_t numColorAttachments;
	bool hasDepth;
};

class RHIRenderPass
{
public:
	RHIRenderPass(RHIDevice* device, const RHIRenderPassInfo& info);
	virtual ~RHIRenderPass();
	uint32_t getID() { return mID; }
	VkRenderPass getHandle() { return mRenderPass; }
private:
	RHIDevice* mDevice;
	VkRenderPass mRenderPass;
	uint32_t mID;

	static uint32_t sNextValidID;
};
#endif
