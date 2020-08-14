#include "RHIFramebuffer.h"
#include "RHIDevice.h"
#include "RHIRenderPass.h"
#include "RHITexture.h"

RHIFramebuffer::RHIFramebuffer(RHIDevice* device, const RHIFramebufferInfo& info)
	:mDevice(device), mRenderpass(info.renderpass)
{
    mWidth = info.width;
    mHeight = info.height;
	std::vector<VkImageView> attachmentViews;
	for (int i = 0; i < (int)info.numColorAttachments; i++)
	{
		attachmentViews.push_back(info.color[i]->getView());
	}

	if(info.hasDepth)
		attachmentViews.push_back(info.depth->getView());

	VkFramebufferCreateInfo framebufferInfo;
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.pNext = nullptr;
	framebufferInfo.flags = 0;
	framebufferInfo.attachmentCount = attachmentViews.size();
	framebufferInfo.pAttachments = attachmentViews.data();
	framebufferInfo.width = info.width;
	framebufferInfo.height = info.height;
	framebufferInfo.layers = info.layers;
	framebufferInfo.renderPass = mRenderpass->getHandle();

	CHECK_VKRESULT(vkCreateFramebuffer(mDevice->getDevice(), &framebufferInfo, nullptr, &mFramebuffer));
}

RHIFramebuffer::~RHIFramebuffer()
{
	vkDestroyFramebuffer(mDevice->getDevice(), mFramebuffer, nullptr);
}
