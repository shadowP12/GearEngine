#include "SwapChain.h"

void SwapChain::createSwapChain(const VkExtent2D &extent)
{
	mSwapChainExtent = extent;
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport();

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = VulkanContext::instance().getSurface();
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = { VulkanContext::instance().getGraphicsFamily(), VulkanContext::instance().getPresentFamily() };

	if (queueFamilyIndices[0] != queueFamilyIndices[1])
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(VulkanContext::instance().getDevice(), &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(VulkanContext::instance().getDevice(), mSwapChain, &imageCount, nullptr);
	mSwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(VulkanContext::instance().getDevice(), mSwapChain, &imageCount, mSwapChainImages.data());

	mSwapChainImageFormat = surfaceFormat.format;

	//imageview
	mSwapChainImageViews.resize(mSwapChainImages.size());

	for (uint32_t i = 0; i < mSwapChainImages.size(); i++)
	{
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = mSwapChainImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = mSwapChainImageFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(VulkanContext::instance().getDevice(), &viewInfo, nullptr, &mSwapChainImageViews[i]) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create image view!");
		}
	}
}

void SwapChain::createFrameBuffer(std::shared_ptr<RenderPass> renderPass)
{

	mFrameBuffers.resize(mSwapChainImages.size());

	for (size_t i = 0; i < mSwapChainImages.size(); i++)
	{
		FramebufferDesc desc;
		desc.width = mSwapChainExtent.width;
		desc.height = mSwapChainExtent.height;
		desc.layers = 1;
		desc.numSamples = 1;
		desc.offscreen = false;
		desc.color[0].format = mSwapChainImageFormat;
		desc.color[0].imageView = mSwapChainImageViews[i];
		desc.color[0].init = true;
		desc.depth.format = VK_FORMAT_D32_SFLOAT;
		desc.depth.imageView = mDepth->getView();
		desc.depth.init = true;

		mFrameBuffers[i] = new Framebuffer(desc);
	}
}

void SwapChain::createDepthMap()
{
	VkImageCreateInfo depthImageCI = {};
	depthImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depthImageCI.pNext = nullptr;
	depthImageCI.flags = 0;
	depthImageCI.format = VK_FORMAT_D32_SFLOAT;
	depthImageCI.imageType = VK_IMAGE_TYPE_2D;
	depthImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	depthImageCI.extent = { mSwapChainExtent.width, mSwapChainExtent.height, 1 };
	depthImageCI.mipLevels = 1;
	depthImageCI.arrayLayers = 1;
	depthImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	depthImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	depthImageCI.queueFamilyIndexCount = 0;
	depthImageCI.pQueueFamilyIndices = nullptr;

	IMAGE_DESC depthDesc;
	depthDesc.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT ;
	depthDesc.numFaces = 1;
	depthDesc.type = TextureType::TEX_TYPE_2D;
	depthDesc.layout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthDesc.numMipLevels = 1;
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

SwapChainSupportDetails SwapChain::querySwapChainSupport()
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanContext::instance().getPhyDevice(), VulkanContext::instance().getSurface(), &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanContext::instance().getPhyDevice(), VulkanContext::instance().getSurface(), &formatCount, nullptr);

	if (formatCount != 0) 
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanContext::instance().getPhyDevice(), VulkanContext::instance().getSurface(), &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanContext::instance().getPhyDevice(), VulkanContext::instance().getSurface(), &presentModeCount, nullptr);

	if (presentModeCount != 0) 
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanContext::instance().getPhyDevice(), VulkanContext::instance().getSurface(), &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) 
	{
		return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : availableFormats) 
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) 
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}