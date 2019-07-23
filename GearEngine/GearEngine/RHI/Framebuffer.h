#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H
#include "VulkanContext.h"
#include "RenderPass.h"
#include "Image.h"
/**
  framebuffer的layers默认为0(足够应付大多情况)
*/
struct AttachmentDesc
{
	VkImageView imageView;
	VkFormat format = VK_FORMAT_UNDEFINED;
	bool init = false;
};

struct FramebufferDesc
{
	AttachmentDesc color[8];
	AttachmentDesc depth;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t layers = 0;
	uint32_t numSamples = 0;//todo
	bool offscreen = false;
};

struct FramebufferAttachment
{
	VkImageView imageView;
	uint32_t baseLayer = 0;
	uint32_t index = 0;
	VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};

class Framebuffer
{
public:
	Framebuffer(const FramebufferDesc& desc)
	{
		mId = sNextId++;

		uint32_t attachmentIdx = 0;
		for (uint32_t i = 0; i < 8; i++)
		{
			if (!desc.color[i].init)
				continue;

			VkAttachmentDescription& attachmentDesc = mAttachments[attachmentIdx];
			attachmentDesc.flags = 0;
			attachmentDesc.format = desc.color[i].format;
			attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			if (desc.offscreen)
				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			else
				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			mColorAttachments[attachmentIdx].baseLayer = 1; //todo
			mColorAttachments[attachmentIdx].imageView = desc.color[i].imageView;
			mColorAttachments[attachmentIdx].finalLayout = attachmentDesc.finalLayout;
			mColorAttachments[attachmentIdx].index = i;

			VkAttachmentReference& ref = mColorReferences[attachmentIdx];
			ref.attachment = attachmentIdx;
			ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			//todo
			mAttachmentViews[attachmentIdx] = desc.color[i].imageView;

			attachmentIdx++;
		}

		mNumColorAttachments = attachmentIdx;
		mHasDepth = desc.depth.init;

		if (mHasDepth)
		{
			VkAttachmentDescription& attachmentDesc = mAttachments[attachmentIdx];
			attachmentDesc.flags = 0;
			attachmentDesc.format = desc.depth.format;
			attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			mDepthStencilAttachment.baseLayer = 0;
			mDepthStencilAttachment.imageView = desc.depth.imageView;
			mDepthStencilAttachment.finalLayout = attachmentDesc.finalLayout;
			mDepthStencilAttachment.index = 0;

			VkAttachmentReference& ref = mDepthReference;
			ref.attachment = attachmentIdx;
			ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			mAttachmentViews[attachmentIdx] = desc.depth.imageView;

			attachmentIdx++;
		}

		mNumAttachments = attachmentIdx;

		mSubpassDesc.flags = 0;
		mSubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		mSubpassDesc.colorAttachmentCount = mNumColorAttachments;
		mSubpassDesc.inputAttachmentCount = 0;
		mSubpassDesc.pInputAttachments = nullptr;
		mSubpassDesc.preserveAttachmentCount = 0;
		mSubpassDesc.pPreserveAttachments = nullptr;
		mSubpassDesc.pResolveAttachments = nullptr;

		if (mNumColorAttachments > 0)
			mSubpassDesc.pColorAttachments = mColorReferences;
		else
			mSubpassDesc.pColorAttachments = nullptr;

		if (mHasDepth)
			mSubpassDesc.pDepthStencilAttachment = &mDepthReference;
		else
			mSubpassDesc.pDepthStencilAttachment = nullptr;

		// Subpass dependencies for layout transitions
		mDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		mDependencies[0].dstSubpass = 0;
		mDependencies[0].srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		mDependencies[0].dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		mDependencies[0].srcAccessMask = 0;
		mDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
		mDependencies[0].dependencyFlags = 0;

		mDependencies[1].srcSubpass = 0;
		mDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		mDependencies[1].srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		mDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		mDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
		mDependencies[1].dstAccessMask = 0;
		mDependencies[1].dependencyFlags = 0;

		mRenderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		mRenderPassCI.pNext = nullptr;
		mRenderPassCI.flags = 0;
		mRenderPassCI.attachmentCount = mNumAttachments;
		mRenderPassCI.pAttachments = mAttachments;
		mRenderPassCI.subpassCount = 1;
		mRenderPassCI.pSubpasses = &mSubpassDesc;
		mRenderPassCI.dependencyCount = 2;
		mRenderPassCI.pDependencies = mDependencies;

		if (vkCreateRenderPass(VulkanContext::instance().getDevice(), &mRenderPassCI, nullptr, &mRenderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		}

		mFramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		mFramebufferCI.pNext = nullptr;
		mFramebufferCI.flags = 0;
		mFramebufferCI.renderPass = mRenderPass;
		mFramebufferCI.attachmentCount = mNumAttachments;
		mFramebufferCI.pAttachments = mAttachmentViews;
		mFramebufferCI.width = desc.width;
		mFramebufferCI.height = desc.height;
		mFramebufferCI.layers = desc.layers;



		if (vkCreateFramebuffer(VulkanContext::instance().getDevice(), &mFramebufferCI, nullptr, &mFramebuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
	~Framebuffer()
	{
		vkDestroyFramebuffer(VulkanContext::instance().getDevice(), mFramebuffer, nullptr);
		vkDestroyRenderPass(VulkanContext::instance().getDevice(), mRenderPass, nullptr);
	}

	VkFramebuffer getFrameBuffer() { return mFramebuffer; }
	VkRenderPass getRenderPass() { return mRenderPass; }
private:
	VkFramebuffer mFramebuffer;
	VkRenderPass mRenderPass;
	FramebufferAttachment mColorAttachments[8];
	FramebufferAttachment mDepthStencilAttachment;
	uint32_t mNumAttachments;
	uint32_t mNumColorAttachments;
	bool mHasDepth;
	uint32_t mId;
	mutable VkAttachmentDescription mAttachments[9];
	mutable VkImageView mAttachmentViews[9];
	mutable VkAttachmentReference mColorReferences[8];
	mutable VkAttachmentReference mDepthReference;
	mutable VkSubpassDescription mSubpassDesc;
	mutable VkSubpassDependency mDependencies[2];
	mutable VkRenderPassCreateInfo mRenderPassCI;
	mutable VkFramebufferCreateInfo mFramebufferCI;

	static uint32_t sNextId;
};

#endif