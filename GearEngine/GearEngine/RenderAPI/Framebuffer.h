#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H
#include "VulkanContext.h"
#include "RenderPass.h"
#include "Image.h"
class FrameBuffer
{
public:
	FrameBuffer(uint32_t numColor, bool hasDepth, std::vector<VkFormat>& formats,uint32_t width, uint32_t height)
	{
		mWidth = width;
		mHeight = height;
		mNumColor = numColor;
		mHasDepth = hasDepth;
		createImages(formats);
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