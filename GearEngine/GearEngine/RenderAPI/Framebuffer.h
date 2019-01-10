#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H
#include "VulkanContext.h"
#include "RenderPass.h"
#include "Image.h"
class FrameBuffer
{
public:
	FrameBuffer(VkRenderPass renderPass, uint32_t numColor, bool hasDepth, std::vector<VkFormat>& formats,uint32_t width, uint32_t height)
	{
		mWidth = width;
		mHeight = height;
		mNumColor = numColor;
		mHasDepth = hasDepth;
		createImages(formats);
		std::vector<VkImageView> attachments;
		for (uint32_t i = 0; i < numColor; i++)
		{
			attachments.push_back(mColors[i]->getView());
		}
		if (hasDepth)
		{
			attachments.push_back(mDepth->getView());
		}
		VkFramebufferCreateInfo framebufferCI = {};
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.renderPass = renderPass;
		framebufferCI.attachmentCount = 2;
		framebufferCI.pAttachments = attachments.data();
		framebufferCI.width = mWidth;
		framebufferCI.height = mHeight;
		framebufferCI.layers = 1;

		if (vkCreateFramebuffer(VulkanContext::instance().getDevice(), &framebufferCI, nullptr, &mFramebuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
	~FrameBuffer()
	{
		vkDestroyFramebuffer(VulkanContext::instance().getDevice(), mFramebuffer, nullptr);
		for (uint32_t i = 0; i < 8; i++)
		{
			if (mColors[i])
			{
				mColors[i]->destroy();
			}
		}
		mDepth->destroy();
	}
	Image* getColor(uint32_t idx)
	{
		return mColors[idx];
	}
	Image* getDepth() { return mDepth; }
private:
	void createImages(std::vector<VkFormat>& formats);
private:
	VkFramebuffer mFramebuffer;
	Image* mColors[8];
	Image* mDepth;
	uint32_t mWidth;
	uint32_t mHeight;
	uint32_t mNumColor;
	bool mHasDepth;
};

#endif