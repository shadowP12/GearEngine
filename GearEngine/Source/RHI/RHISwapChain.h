#ifndef RHI_SWAP_CHAIN_H
#define RHI_SWAP_CHAIN_H
#include "RHIDefine.h"
#include <vector>
/**
 交换链不提供重构功能,当窗口尺寸发生改变时应重新创建一个交换链
*/
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class RHIDevice;

class RHISwapChain
{
public:
	RHISwapChain(RHIDevice* device, VkSurfaceKHR surface, uint32_t width, uint32_t height);
	~RHISwapChain();
	VkSwapchainKHR getHandle() { return mSwapChain; }
private:
	SwapChainSupportDetails querySwapChainSupport();
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
private:
	RHIDevice* mDevice;
	VkSurfaceKHR mSurface;
	VkSwapchainKHR mSwapChain;
	std::vector<VkImage> mSwapChainImages;
	std::vector<VkImageView> mSwapChainImageViews;
	VkFormat mSwapChainImageFormat;
	uint32_t mWidth;
	uint32_t mHeight;
};

#endif
