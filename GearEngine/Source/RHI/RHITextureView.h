#ifndef RHI_TEXTURE_VIEW
#define RHI_TEXTURE_VIEW
#include "RHIDefine.h"
#include "RHITexture.h"

struct RHITextureViewInfo
{
	RHITexture* texture;
	VkFormat format;
	VkImageAspectFlags aspectMask;
	uint32_t baseMipLevel;
	uint32_t levelCount;
	uint32_t baseArrayLayer;
	uint32_t layerCount;
};

class RHIDevice;
class RHITextureView
{
public:
	RHITextureView(RHIDevice* device, const RHITextureViewInfo& textureViewDesc);
	RHITextureView(RHIDevice* device, VkImageView view);
	~RHITextureView();
	VkImageView getHandle() { return mImageView; }
private:
	RHIDevice* mDevice;
	RHITexture* mTexture;
	VkImageView mImageView;
};
#endif
