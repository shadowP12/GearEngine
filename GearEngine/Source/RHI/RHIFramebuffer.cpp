#include "RHIFramebuffer.h"
#include "RHIDevice.h"
#include "RHIRenderPass.h"
#include "RHITextureView.h"
RHIFramebuffer::RHIFramebuffer(RHIDevice* device, const RHIFramebufferInfo& info)
	:mDevice(device), mRenderpass(info.renderpass)
{
	std::vector<VkImageView> attachmentViews;
	for (int i = 0; i < info.numColorAttachments; i++)
	{
		attachmentViews.push_back(info.color[i]->getHandle());
	}

	if(info.hasDepth)
		attachmentViews.push_back(info.depth->getHandle());

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

	if (vkCreateFramebuffer(mDevice->getDevice(), &framebufferInfo, nullptr, &mFramebuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create framebuffer!");
	}
}

RHIFramebuffer::~RHIFramebuffer()
{
}
