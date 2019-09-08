#ifndef TEXTURE_2D_H
#define TEXTURE_2D_H
#include "Resource/Resource.h"
#include "RHI//RHITexture.h"
#include "RHI//RHITextureView.h"

class Texture2D : public Resource
{
public:
	Texture2D();
	~Texture2D();
	void writeData(void *pixelData, uint32_t size);
private:
	void setImageLayout(const VkFormat &format, const VkImageLayout &oldLayout, const VkImageLayout &newLayout);
	void copyBufferToImage(const uint32_t &width, const uint32_t &height, const VkBuffer &buffer, const VkImage &image);
	void createSampler(const VkFilter &filter = VK_FILTER_NEAREST, const VkSamplerAddressMode &addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
private:
	RHITexture* mTexture;
	RHITextureView* mTextureView;
	VkSampler mSampler;
	uint32_t mWidth, mHeight;
	uint32_t mNumMipLevels;
	uint32_t mNumFaces;
};
#endif