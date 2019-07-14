#include "RHISwapChain.h"
#include "RHIDevice.h"

RHISwapChain::RHISwapChain(RHIDevice* device, VkSurfaceKHR surface, uint32_t width, uint32_t height)
	:mDevice(device),mSurface(surface),mWidth(width),mHeight(height)
{
	VkExtent2D swapChainExtent;
	swapChainExtent.width = mWidth;
	swapChainExtent.height = mHeight;

	SwapChainSupportDetails swapChainSupport = querySwapChainSupport();

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainInfo = {};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = mSurface;
	swapchainInfo.minImageCount = imageCount;
	swapchainInfo.imageFormat = surfaceFormat.format;
	swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainInfo.imageExtent = swapChainExtent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainInfo.queueFamilyIndexCount = 0;
	swapchainInfo.pQueueFamilyIndices = nullptr;
	swapchainInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = presentMode;
	swapchainInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(mDevice->getDevice(), &swapchainInfo, nullptr, &mSwapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	//get image
	vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &imageCount, nullptr);
	mSwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &imageCount, mSwapChainImages.data());
	mSwapChainImageFormat = surfaceFormat.format;

	//get image view
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

		if (vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &mSwapChainImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image view!");
		}
	}
}

RHISwapChain::~RHISwapChain()
{
	for (size_t i = 0; i < mSwapChainImageViews.size(); i++)
	{
		vkDestroyImageView(mDevice->getDevice(), mSwapChainImageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(mDevice->getDevice(), mSwapChain, nullptr);
}



SwapChainSupportDetails RHISwapChain::querySwapChainSupport()
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->getPhyDevice(), mSurface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhyDevice, mSurface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhyDevice(), mSurface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhyDevice(), mSurface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhyDevice(), mSurface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR RHISwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR RHISwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
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

void RHISwapChain::createSwapChain(const VkExtent2D & extent)
{

}

