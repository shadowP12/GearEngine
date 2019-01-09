#ifndef TEXTURE_H
#define TEXTURE_H
#include "../RenderAPI/Image.h"
#include "../Resource/Resource.h"
#include "../RenderAPI/Buffer.h"
class Texture : public Resource
{
public:
	Texture();
	~Texture();
	void init(VkImageUsageFlags usage, VkFormat format, VkSampleCountFlagBits samples, VkImageAspectFlags imageAspect);
	void createSampler(const VkFilter &filter = VK_FILTER_NEAREST, const VkSamplerAddressMode &addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
	void setImageLayout(const VkImageLayout &newLayout, const uint32_t &mipLevels, const uint32_t &baseArrayLayer, const uint32_t &layerCount);
	void copyBufferToImage(const uint32_t &width, const uint32_t &height, const VkBuffer &buffer, const VkImage &image);
	void writeData(void *pixelData);
	VkSampler getSampler() { return mSampler; }
	VkImage getImage() { return mImage->getImage(); }
	VkImageView getView() { return mImage->getView(); }
	VkImageLayout getLayout() { return mDesc.layout; }
private:
	void createImage();
	Buffer* createStaging();
private:
	Image* mImage;
	VkImageCreateInfo mImageCI;
	VkSampler mSampler;
	IMAGE_DESC mDesc;
	uint32_t mWidth, mHeight;
	uint32_t mNumMipLevels;
	uint32_t mNumFaces;
};
#endif