#include "Framebuffer.h"

void FrameBuffer::createImages(std::vector<VkFormat>& formats)
{
	VkImageCreateInfo colorImageCI = {};
	colorImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	colorImageCI.pNext = nullptr;
	colorImageCI.flags = 0;
	colorImageCI.imageType = VK_IMAGE_TYPE_2D;
	colorImageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT| VK_IMAGE_USAGE_SAMPLED_BIT;
	colorImageCI.extent = { mWidth, mHeight, 1 };
	colorImageCI.mipLevels = 1;
	colorImageCI.arrayLayers = 1;
	colorImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	colorImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	colorImageCI.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	colorImageCI.queueFamilyIndexCount = 0;
	colorImageCI.pQueueFamilyIndices = nullptr;

	IMAGE_DESC colorDesc;
	colorDesc.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	colorDesc.numFaces = 1;
	colorDesc.type = TextureType::TEX_TYPE_2D;
	colorDesc.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorDesc.numFaces = 1;
	colorDesc.numMipLevels = 0;
	
	for (uint32_t i = 0; i < mNumColor; i++)
	{
		colorDesc.format = formats[i];
		VkImage image;

		if (vkCreateImage(VulkanContext::instance().getDevice(), &colorImageCI, nullptr, &image))
		{
			throw std::runtime_error("failed to create image !");
		}
		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(VulkanContext::instance().getDevice(), image, &memoryRequirements);

		VkMemoryPropertyFlags flags = (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);//todo

		VkMemoryAllocateInfo memoryAllocateInfo = {};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = VulkanContext::instance().findMemoryType(memoryRequirements.memoryTypeBits, flags);

		VkDeviceMemory memory;
		if (vkAllocateMemory(VulkanContext::instance().getDevice(), &memoryAllocateInfo, nullptr, &memory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(VulkanContext::instance().getDevice(), image, memory, 0);
		colorDesc.image = image;

		mColors[i] = VulkanContext::instance().getResourceManager()->create<Image>(colorDesc, memory);
	}

	if (mHasDepth)
	{
		VkImageCreateInfo depthImageCI = {};
		depthImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		depthImageCI.pNext = nullptr;
		depthImageCI.flags = 0;
		depthImageCI.imageType = VK_IMAGE_TYPE_2D;
		depthImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		depthImageCI.extent = { mWidth, mHeight, 1 };
		depthImageCI.mipLevels = 1;
		depthImageCI.arrayLayers = 1;
		depthImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		depthImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		depthImageCI.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		depthImageCI.queueFamilyIndexCount = 0;
		depthImageCI.pQueueFamilyIndices = nullptr;

		IMAGE_DESC depthDesc;
		depthDesc.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		depthDesc.numFaces = 1;
		depthDesc.type = TextureType::TEX_TYPE_2D;
		depthDesc.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthDesc.numFaces = 1;
		depthDesc.numMipLevels = 0;
		depthDesc.format = VK_FORMAT_D32_SFLOAT;

		VkImage depthImage;

		if (vkCreateImage(VulkanContext::instance().getDevice(), &depthImageCI, nullptr, &depthImage))
		{
			throw std::runtime_error("failed to create image !");
		}
		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(VulkanContext::instance().getDevice(), depthImage, &memoryRequirements);

		VkMemoryPropertyFlags flags = (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);//todo

		VkMemoryAllocateInfo memoryAllocateInfo = {};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = VulkanContext::instance().findMemoryType(memoryRequirements.memoryTypeBits, flags);

		VkDeviceMemory memory;
		if (vkAllocateMemory(VulkanContext::instance().getDevice(), &memoryAllocateInfo, nullptr, &memory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(VulkanContext::instance().getDevice(), depthImage, memory, 0);
		depthDesc.image = depthImage;

		mDepth = VulkanContext::instance().getResourceManager()->create<Image>(depthDesc, memory);
	}
}
