#ifndef RHI_TEXTURE_VIEW
#define RHI_TEXTURE_VIEW
#include "RHIDefine.h"
#include "RHITexture.h"

struct TextureViewDesc
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
	RHITextureView(RHIDevice* device, const TextureViewDesc& textureViewDesc);
	~RHITextureView();
	VkImageView getHandle() { return mImageView; }
private:
	RHIDevice* mDevice;
	RHITexture* mTexture;
	VkImageView mImageView;
};
#endif
