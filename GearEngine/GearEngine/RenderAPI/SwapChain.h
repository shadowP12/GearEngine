#ifndef SWAP_CHAIN_H
#define SWAP_CHAIN_H
#include "Framebuffer.h"
#include "Image.h"

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class SwapChain
{
public:
	SwapChain(const VkExtent2D& extent, std::shared_ptr<RenderPass> renderPass)
	{
		createSwapChain(extent);
		createDepthMap();
		createFrameBuffer(renderPass);
	}
	~SwapChain()
	{
		cleanupSwapChain();
	}
	void reCreateSwapChain(const VkExtent2D& extent, std::shared_ptr<RenderPass> renderPass)
	{
		cleanupSwapChain();
		createSwapChain(extent);
		createDepthMap();
		createFrameBuffer(renderPass);
	}
	VkSwapchainKHR getSwapchain() { return mSwapChain; }
	Framebuffer* getFrameBuffer(uint32_t idx) { return mFrameBuffers[idx]; }
	VkFormat getFormat() { return mSwapChainImageFormat; }
	uint32_t getFrameBufferSize() { return mFrameBuffers.size(); }
private:
	void createSwapChain(const VkExtent2D &extent);
	void createFrameBuffer(std::shared_ptr<RenderPass> renderPass);
	void createDepthMap();
	SwapChainSupportDetails querySwapChainSupport();
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
	void cleanupSwapChain() 
	{
		if (mDepth)
			mDepth->destroy();

		for (size_t i = 0; i < mSwapChainImageViews.size(); i++)
		{
			vkDestroyImageView(VulkanContext::instance().getDevice(), mSwapChainImageViews[i], nullptr);
		}

		for (size_t i = 0; i < mFrameBuffers.size(); i++)
		{
			delete mFrameBuffers[i];
		}

		vkDestroySwapchainKHR(VulkanContext::instance().getDevice(), mSwapChain, nullptr);
	}
private:
	VkSwapchainKHR mSwapChain;
	std::vector<VkImage> mSwapChainImages;
	std::vector<VkImageView> mSwapChainImageViews;
	std::vector<Framebuffer*> mFrameBuffers;
	VkFormat mSwapChainImageFormat;
	VkExtent2D mSwapChainExtent;
	Image* mDepth;
};

#endif
