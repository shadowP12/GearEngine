#include "RHITextureView.h"
#include "RHIDevice.h"
RHITextureView::RHITextureView(RHIDevice* device, const TextureViewDesc& textureViewDesc)
	:mDevice(device),mTexture(textureViewDesc.texture)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = mTexture->getHandle();
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = textureViewDesc.format;
	viewInfo.subresourceRange.aspectMask = textureViewDesc.aspectMask;
	viewInfo.subresourceRange.baseMipLevel = textureViewDesc.baseMipLevel;
	viewInfo.subresourceRange.levelCount = textureViewDesc.levelCount;
	viewInfo.subresourceRange.baseArrayLayer = textureViewDesc.baseArrayLayer;
	viewInfo.subresourceRange.layerCount = textureViewDesc.layerCount;

	if (vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &mImageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image view!");
	}
}

RHITextureView::~RHITextureView()
{
	vkDestroyImageView(mDevice->getDevice(), mImageView, nullptr);
}