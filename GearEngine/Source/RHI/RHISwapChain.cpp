#include "RHISwapChain.h"
#include "RHIRenderPass.h"
#include "RHIFramebuffer.h"
#include "RHITexture.h"
#include "RHIRenderPass.h"
#include "RHIDevice.h"
#include "RHISynchronization.h"
#include "RHIQueue.h"

RHISwapChain::RHISwapChain(RHIDevice* device, const RHISwapChainInfo& info)
	:mDevice(device),mWidth(info.width),mHeight(info.height)
{
    VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.pNext = NULL;
    surfaceInfo.flags = 0;
    surfaceInfo.hinstance = ::GetModuleHandle(NULL);
    surfaceInfo.hwnd = (HWND)info.windowHandle;
    CHECK_VKRESULT(vkCreateWin32SurfaceKHR(device->getInstance(), &surfaceInfo, nullptr, &mSurface));

    VkBool32 supportsPresent;
    vkGetPhysicalDeviceSurfaceSupportKHR(mDevice->getGPU(), mDevice->getGraphicsQueue()->getFamilyIndex(), mSurface, &supportsPresent);
    if (!supportsPresent)
    {
        printf("cannot find a graphics queue that also supports present operations.\n");
    }

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

	CHECK_VKRESULT(vkCreateSwapchainKHR(mDevice->getDevice(), &swapchainInfo, nullptr, &mSwapChain));

	//get image
	vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &imageCount, nullptr);
	mSwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &imageCount, mSwapChainImages.data());
	mSwapChainImageFormat = surfaceFormat.format;

	// create textures
	for (uint32_t i = 0; i < mSwapChainImages.size(); i++)
	{
	    RHITexture* colorTexture = new RHITexture(mDevice, this, i);
	    mColorTextures.push_back(colorTexture);

        RHITextureInfo texInfo;
        texInfo.width = mWidth;
        texInfo.height = mHeight;
        texInfo.depth = 1;
        texInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
        texInfo.arrayLayers = 1;
        texInfo.mipLevels = 1;
        texInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        texInfo.descriptors = DESCRIPTOR_TYPE_TEXTURE;
        texInfo.colorAtt = false;
        texInfo.depthStencilAtt = true;
	    RHITexture* depthStencilTexture = new RHITexture(mDevice, texInfo);
	    mDepthStencilTextures.push_back(depthStencilTexture);
	}

	// create renderpass
    {
        RHIColorAttachmentInfo color;
        color.sampleCount = VK_SAMPLE_COUNT_1_BIT;
        color.format = VK_FORMAT_B8G8R8A8_UNORM;
//        color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//        color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        RHIDepthStencilAttachmentInfo depthStencil;
        depthStencil.sampleCount = VK_SAMPLE_COUNT_1_BIT;
        depthStencil.format = VK_FORMAT_D24_UNORM_S8_UINT;

        RHIRenderPassInfo passInfo;
        passInfo.numColorAttachments = 1;
        passInfo.color[0] = color;
        passInfo.hasDepth = true;
        passInfo.depthStencil = depthStencil;
        mRenderPass = new RHIRenderPass(mDevice, passInfo);
    }

    // create framebuffer
	for (uint32_t i = 0; i < mSwapChainImages.size(); i++)
	{
		RHIFramebufferInfo fbInfo;
		fbInfo.color[0] = mColorTextures[i];
		fbInfo.depth = nullptr;
		fbInfo.width = mWidth;
		fbInfo.height = mHeight;
		fbInfo.renderpass = mRenderPass;
		fbInfo.layers = 1;
		fbInfo.numColorAttachments = 1;
		fbInfo.hasDepth = true;
		fbInfo.depth = mDepthStencilTextures[i];

		RHIFramebuffer* fb = new RHIFramebuffer(mDevice, fbInfo);
		mFramebuffers.push_back(fb);
	}
}

RHISwapChain::~RHISwapChain()
{
	for (int i = 0; i < (int)mColorTextures.size(); i++)
	{
		delete mColorTextures[i];
	}
	mColorTextures.clear();

    for (int i = 0; i < (int)mDepthStencilTextures.size(); i++)
    {
        delete mDepthStencilTextures[i];
    }
    mDepthStencilTextures.clear();

	if (mRenderPass)
		delete mRenderPass;

	for (int i = 0; i < (int)mFramebuffers.size(); i++)
	{
		delete mFramebuffers[i];
	}

	vkDestroySwapchainKHR(mDevice->getDevice(), mSwapChain, nullptr);
    vkDestroySurfaceKHR(mDevice->getInstance(), mSurface, nullptr);
}

void RHISwapChain::acquireNextImage(RHISemaphore *signalSemaphore, RHIFence* inFence, uint32_t &imageIndex)
{
    VkResult vkRes = {};
    VkSemaphore semaphore = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;

    if(inFence)
        fence = inFence->getHandle();
    if(signalSemaphore)
        semaphore = signalSemaphore->getHandle();

    vkRes = vkAcquireNextImageKHR(mDevice->getDevice(), mSwapChain, UINT64_MAX, semaphore, fence, &imageIndex);
    if (vkRes == VK_ERROR_OUT_OF_DATE_KHR)
    {
        printf("swapChain out of date\n");
        imageIndex = -1;
        if (inFence)
        {
            VkFence fences[] = { fence };
            vkResetFences(mDevice->getDevice(), 1, fences);
        }
    }
}

SwapChainSupportDetails RHISwapChain::querySwapChainSupport()
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->getGPU(), mSurface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getGPU(), mSurface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getGPU(), mSurface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getGPU(), mSurface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getGPU(), mSurface, &presentModeCount, details.presentModes.data());
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

RHIFramebuffer* RHISwapChain::getFramebuffer(uint32_t index)
{
	if (index < 0 || index >= mFramebuffers.size())
	{
		return nullptr;
	}

	return mFramebuffers[index];
}

RHITexture* RHISwapChain::getColorTexture(uint32_t index)
{
    if (index < 0 || index >= mColorTextures.size())
    {
        return nullptr;
    }

    return mColorTextures[index];
}

RHITexture* RHISwapChain::getDepthStencilTexture(uint32_t index)
{
    if (index < 0 || index >= mDepthStencilTextures.size())
    {
        return nullptr;
    }

    return mDepthStencilTextures[index];
}
