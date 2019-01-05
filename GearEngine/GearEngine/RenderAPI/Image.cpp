#include "Image.h"
#include "VulkanContext.h"

Image::Image(VulkanResourceManager * manager, VULKAN_IMAGE_DESC desc, VkDeviceMemory memory)
	:VulkanResource(manager), mImage(desc.image), mNumFaces(desc.numFaces), mNumMipLevels(desc.numMipLevels)
{
	mImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	mImageViewCI.image = mImage;
	mImageViewCI.format = desc.format;

	switch (desc.type)
	{
	case TextureType::TEX_TYPE_1D:
		mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_1D;
		break;
	default:
	case TextureType::TEX_TYPE_2D:
		mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		break;
	case TextureType::TEX_TYPE_3D:
		mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_3D;
		break;
	case TextureType::TEX_TYPE_CUBE_MAP:
		mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		break;
	}
	createView(desc.aspectFlags);
}

Image::~Image()
{
	vkDestroyImageView(VulkanContext::instance().getDevice(), mImageView, nullptr);
	vkDestroyImage(VulkanContext::instance().getDevice(), mImage, nullptr);
	vkFreeMemory(VulkanContext::instance().getDevice(), mBufferMemory, nullptr);
}

void Image::createView(VkImageAspectFlags aspectMask)
{
	VkImageViewType oldViewType = mImageViewCI.viewType;
	switch (oldViewType)
	{
	case VK_IMAGE_VIEW_TYPE_CUBE:
		if (mNumFaces == 1)
			mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		else if (mNumFaces % 6 == 0)
		{
			if (mNumFaces > 6)
				mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
		}
		else
			mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		break;
	case VK_IMAGE_VIEW_TYPE_1D:
		if (mNumFaces > 1)
			mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		break;
	case VK_IMAGE_VIEW_TYPE_2D:
	case VK_IMAGE_VIEW_TYPE_3D:
		if (mNumFaces > 1)
			mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		break;
	default:
		break;
	}

	mImageViewCI.subresourceRange.aspectMask = aspectMask;
	mImageViewCI.subresourceRange.baseMipLevel = 0;
	mImageViewCI.subresourceRange.levelCount = mNumMipLevels;
	mImageViewCI.subresourceRange.baseArrayLayer = 0;
	mImageViewCI.subresourceRange.layerCount = mNumFaces;

	if (vkCreateImageView(VulkanContext::instance().getDevice(), &mImageViewCI, nullptr, &mImageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture image view!");
	}
}
