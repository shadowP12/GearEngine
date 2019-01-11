#ifndef TEXTURE_H
#define TEXTURE_H
#include "../RenderAPI/Image.h"
#include "../Resource/Resource.h"
#include "../RenderAPI/Buffer.h"
class TextureImporter;
class Texture : public Resource
{
public:
	Texture();
	~Texture();
	void init(uint32_t w, uint32_t h, VkImageUsageFlags usage, VkFormat format, VkSampleCountFlagBits samples, VkImageAspectFlags imageAspect);
	void createSampler(const VkFilter &filter = VK_FILTER_NEAREST, const VkSamplerAddressMode &addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
	void setImageLayout(const VkFormat &format, const VkImageLayout &oldLayout, const VkImageLayout &newLayout);
	void copyBufferToImage(const uint32_t &width, const uint32_t &height, const VkBuffer &buffer, const VkImage &image);
	void writeData(void *pixelData, VkDeviceSize size);
	VkSampler getSampler() { return mSampler; }
	VkImage getImage() { return mImage->getImage(); }
	VkImageView getView() { return mImage->getView(); }
	VkImageLayout getLayout() { return mDesc.layout; }
private:
	void createImage();
	Buffer* createStaging(VkDeviceSize size);
private:
	friend TextureImporter;
	Image* mImage;
	VkImageCreateInfo mImageCI;
	VkSampler mSampler;
	IMAGE_DESC mDesc;
	uint32_t mWidth, mHeight;
	uint32_t mNumMipLevels;
	uint32_t mNumFaces;
};
#endif