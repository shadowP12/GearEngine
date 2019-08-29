#include "RHITexture.h"
#include "RHIDevice.h"

/**
* 稀疏纹理会需要用到flag，但是目前以普通2d纹理为主故设为默认值0
*/

RHITexture::RHITexture(RHIDevice* device, const RHITextureInfo& textureDesc)
	:mDevice(device)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = nullptr;
	imageInfo.flags = 0;
	imageInfo.imageType = textureDesc.type;
	imageInfo.format = textureDesc.format;
	imageInfo.extent = textureDesc.extent;
	imageInfo.mipLevels = textureDesc.mipLevels;
	imageInfo.arrayLayers = textureDesc.arrayLayers;
	imageInfo.samples = textureDesc.samples;
	imageInfo.usage = textureDesc.usage;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = 0;
	imageInfo.pQueueFamilyIndices = nullptr;

	if (vkCreateImage(mDevice->getDevice(), &imageInfo, nullptr, &mImage))
	{
		throw std::runtime_error("failed to create image !");
	}
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(mDevice->getDevice(), mImage, &memoryRequirements);
	//需要修改成自定义flag
	VkMemoryPropertyFlags flags = (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = mDevice->findMemoryType(memoryRequirements.memoryTypeBits, flags);

	if (vkAllocateMemory(mDevice->getDevice(), &memoryAllocateInfo, nullptr, &mMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(mDevice->getDevice(), mImage, mMemory, 0);
}

RHITexture::~RHITexture()
{
	vkDestroyImage(mDevice->getDevice(), mImage, nullptr);
	vkFreeMemory(mDevice->getDevice(), mMemory, nullptr);
}
