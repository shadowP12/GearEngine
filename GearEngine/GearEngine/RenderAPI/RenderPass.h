#ifndef RENDER_PASS_H
#define RENDER_PASS_H
#include "VulkanContext.h"

class RenderPass 
{
public:
	RenderPass(std::vector<VkFormat> colors, bool present, bool hasDepth) 
	{
		mFormats = colors;
		mNumColor = colors.size();
		mHasDepth = hasDepth;
		std::vector<VkAttachmentReference> colorReferences;
		std::vector<VkAttachmentDescription> attachments;
		VkSubpassDescription subpass = {};
		for (uint32_t i = 0; i < mNumColor; ++i)
		{
			attachments[i].format = colors[i];
			attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			if (present)
			{
				attachments[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}
			else
			{
				attachments[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}
			colorReferences.push_back({ i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		}

		if (hasDepth)
		{
			VkAttachmentDescription depthAttachment = {};
			depthAttachment.format = VK_FORMAT_D32_SFLOAT;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachments.push_back(depthAttachment);

			VkAttachmentReference depthReference = {};
			depthReference.attachment = mNumColor;//maybe
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			subpass.flags = 0;
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = mNumColor;
			subpass.inputAttachmentCount = 0;
			subpass.pInputAttachments = nullptr;
			subpass.preserveAttachmentCount = 0;
			subpass.pPreserveAttachments = nullptr;
			subpass.pResolveAttachments = nullptr;

			if (mNumColor > 0)
				subpass.pColorAttachments = colorReferences.data();
			else
				subpass.pColorAttachments = nullptr;

			subpass.pDepthStencilAttachment = &depthReference;

		}
		else
		{
			subpass.flags = 0;
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = mNumColor;
			subpass.inputAttachmentCount = 0;
			subpass.pInputAttachments = nullptr;
			subpass.preserveAttachmentCount = 0;
			subpass.pPreserveAttachments = nullptr;
			subpass.pResolveAttachments = nullptr;

			if (mNumColor > 0)
				subpass.pColorAttachments = colorReferences.data();
			else
				subpass.pColorAttachments = nullptr;

			subpass.pDepthStencilAttachment = nullptr;
		}

		std::vector<VkSubpassDependency> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = 0;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(VulkanContext::instance().getDevice(), &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		}
	}
	~RenderPass() 
	{
		vkDestroyRenderPass(VulkanContext::instance().getDevice(),mRenderPass,nullptr);
	}
	std::vector<VkFormat> getFormats() { return mFormats; }
	uint32_t getNumColor() { return mNumColor; }
	bool getHasDepth() { return mHasDepth; }
	VkRenderPass getRenderPass() { return mRenderPass; }
private:
	uint32_t mNumColor;
	bool mHasDepth;
	std::vector<VkFormat> mFormats;
	VkRenderPass mRenderPass;
};
#endif
