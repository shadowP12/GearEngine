#ifndef RHI_TEXTURE_H
#define RHI_TEXTURE_H
#include "RHIDefine.h"

class RHIDevice;
class RHISwapChain;

struct RHISamplerInfo
{
    VkFilter magFilter = VK_FILTER_LINEAR;
    VkFilter minFilter = VK_FILTER_LINEAR;
    VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
};

class RHISampler
{
public:
    RHISampler(RHIDevice* device, const RHISamplerInfo& info);
    ~RHISampler();
    VkSampler getHandle() { return mSampler; }

private:
    RHIDevice* mDevice;
    VkSampler mSampler;
};

struct RHITextureInfo
{
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    //ResourceState initialState;
    DescriptorType descriptors;
    bool colorAtt = false;
    bool depthStencilAtt = false;
};

class RHITexture
{
public:
	RHITexture(RHIDevice* device, const RHITextureInfo& info);
    RHITexture(RHIDevice* device, RHISwapChain* swapChain, uint32_t idx);
	virtual~RHITexture();
	VkImage getHandle() { return mImage; }
	VkImageView getView() { return mView; }
    VkImageAspectFlags getAspectMask() {return mAspectMask; }
    DescriptorType getDescriptorType() { return mDescriptors; }
    ResourceState getResourceState() { return mCurrentState; }
    void setResourceState(ResourceState state) { mCurrentState = state; }
    uint32_t getWidth() { return mWidth; }
    uint32_t getHeight() { return mHeight; }
    uint32_t getDepth() { return mDepth; }
private:
	friend class RHITextureView;
	RHIDevice* mDevice;
	VkImage mImage;
	VkImageView mView;
    VkImageAspectFlags mAspectMask;
	VkDeviceMemory mMemory;
    DescriptorType mDescriptors;
    ResourceState mCurrentState;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mDepth;
	bool mOwnsImage = true;
};

#endif