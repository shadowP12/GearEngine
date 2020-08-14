#ifndef RHI_SWAP_CHAIN_H
#define RHI_SWAP_CHAIN_H
#include "RHIDefine.h"
#include <vector>

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class RHIDevice;
class RHIRenderPass;
class RHIFramebuffer;
class RHITexture;
class RHISemaphore;
class RHIFence;

struct RHISwapChainInfo {
    void* windowHandle;
    uint32_t width;
    uint32_t height;
};

class RHISwapChain
{
public:
	RHISwapChain(RHIDevice* device, const RHISwapChainInfo& info);
	~RHISwapChain();
	VkSwapchainKHR getHandle() { return mSwapChain; }
	VkImage getImage(uint32_t idx) { return mSwapChainImages[idx]; }
    VkFormat getImageFormat() { return mSwapChainImageFormat; }
	RHIRenderPass* getRenderPass() { return mRenderPass; }
	RHIFramebuffer* getFramebuffer(uint32_t index);
	RHITexture* getColorTexture(uint32_t index);
	RHITexture* getDepthStencilTexture(uint32_t index);
    void acquireNextImage(RHISemaphore* signalSemaphore, RHIFence* inFence, uint32_t& imageIndex);
	uint32_t getWidth() { return mWidth; }
	uint32_t getHeight() { return mHeight; }
private:
	SwapChainSupportDetails querySwapChainSupport();
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
private:
	friend class Window;
	RHIDevice* mDevice;
	RHIRenderPass* mRenderPass;
	std::vector<RHIFramebuffer*> mFramebuffers;
	std::vector<RHITexture*> mColorTextures;
    std::vector<RHITexture*> mDepthStencilTextures;
	VkSurfaceKHR mSurface;
	VkSwapchainKHR mSwapChain;
	std::vector<VkImage> mSwapChainImages;
	VkFormat mSwapChainImageFormat;
	uint32_t mWidth;
	uint32_t mHeight;
};

#endif
