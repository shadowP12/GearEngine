#include "RHITexture.h"
#include "RHIDevice.h"
#include "RHISwapChain.h"
#include <string>

RHISampler::RHISampler(RHIDevice *device, const RHISamplerInfo &info)
    :mDevice(device)
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = info.magFilter;
    samplerInfo.minFilter = info.minFilter;
    samplerInfo.addressModeU = info.addressModeU;
    samplerInfo.addressModeV = info.addressModeV;
    samplerInfo.addressModeW = info.addressModeW;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    CHECK_VKRESULT(vkCreateSampler(mDevice->getDevice(), &samplerInfo, nullptr, &mSampler));
}

RHISampler::~RHISampler()
{
    vkDestroySampler(mDevice->getDevice(), mSampler, nullptr);
}

RHITexture::RHITexture(RHIDevice* device, const RHITextureInfo& info)
	:mDevice(device)
{
    mWidth = info.width;
    mHeight = info.height;
    mDepth = info.depth;
    mOwnsImage = true;
    mDescriptors = info.descriptors;
    mCurrentState = RESOURCE_STATE_UNDEFINED;//info.initialState;
    bool cubemapRequired = false;

    VkImageUsageFlags additionalFlags = 0;
    if (info.colorAtt)
        additionalFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    else if (info.depthStencilAtt)
        additionalFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageType imageType = VK_IMAGE_TYPE_MAX_ENUM;
    if (info.depth > 1)
        imageType = VK_IMAGE_TYPE_3D;
    else if (info.height > 1)
        imageType = VK_IMAGE_TYPE_2D;
    else
        imageType = VK_IMAGE_TYPE_1D;

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = nullptr;
	imageInfo.flags = 0;
	imageInfo.imageType = imageType;
	imageInfo.format = info.format;
	imageInfo.extent.width = info.width;
    imageInfo.extent.height = info.height;
    imageInfo.extent.depth = info.depth;
	imageInfo.mipLevels = info.mipLevels;
	imageInfo.arrayLayers = info.arrayLayers;
	imageInfo.samples = info.samples;
	imageInfo.usage = toVkImageUsage(info.descriptors) | additionalFlags;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = 0;
	imageInfo.pQueueFamilyIndices = nullptr;

    if (DESCRIPTOR_TYPE_TEXTURE_CUBE == (info.descriptors & DESCRIPTOR_TYPE_TEXTURE_CUBE)) {
        cubemapRequired = true;
        imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }
    if (imageType == VK_IMAGE_TYPE_3D)
        imageInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR;

    if ((VK_IMAGE_USAGE_SAMPLED_BIT & imageInfo.usage) || (VK_IMAGE_USAGE_STORAGE_BIT & imageInfo.usage))
    {
        imageInfo.usage |= (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    }

	CHECK_VKRESULT(vkCreateImage(mDevice->getDevice(), &imageInfo, nullptr, &mImage));

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(mDevice->getDevice(), mImage, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = mDevice->findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	CHECK_VKRESULT(vkAllocateMemory(mDevice->getDevice(), &memoryAllocateInfo, nullptr, &mMemory));

	vkBindImageMemory(mDevice->getDevice(), mImage, mMemory, 0);

	// create image view
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    switch (imageType)
    {
        case VK_IMAGE_TYPE_1D:
            viewType = info.arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
            break;
        case VK_IMAGE_TYPE_2D:
            if (cubemapRequired)
                viewType = (info.arrayLayers > 6) ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
            else
                viewType = info.arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
            break;
        case VK_IMAGE_TYPE_3D:
            if (info.arrayLayers > 1)
            {
                assert(false);
            }
            viewType = VK_IMAGE_VIEW_TYPE_3D;
            break;
        default:
            assert(false && "Image Format not supported!");
            break;
    }
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = mImage;
    viewInfo.viewType = viewType;
    viewInfo.format = info.format;
    viewInfo.subresourceRange.aspectMask = toVkAspectMask(info.format);
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = info.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = info.arrayLayers;

    CHECK_VKRESULT(vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &mView));

    mAspectMask = viewInfo.subresourceRange.aspectMask;
}

RHITexture::RHITexture(RHIDevice* device, RHISwapChain* swapChain, uint32_t idx)
    :mDevice(device)
{
    mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
    mCurrentState = RESOURCE_STATE_UNDEFINED;
    mWidth = swapChain->getWidth();
    mHeight = swapChain->getHeight();
    mDepth = 1;
    mOwnsImage = false;
    mAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    mImage = swapChain->getImage(idx);
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = swapChain->getImage(idx);
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = swapChain->getImageFormat();
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    CHECK_VKRESULT(vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &mView));
}

RHITexture::~RHITexture()
{
    if(mOwnsImage) {
        vkDestroyImage(mDevice->getDevice(), mImage, nullptr);
        vkFreeMemory(mDevice->getDevice(), mMemory, nullptr);
    }
    vkDestroyImageView(mDevice->getDevice(), mView, nullptr);
}
