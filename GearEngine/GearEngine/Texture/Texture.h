#ifndef TEXTURE_H
#define TEXTURE_H
#include "../RenderAPI/Image.h"
#include "../Resource/Resource.h"
#include "../RenderAPI/Buffer.h"
class Texture
{
public:
	Texture();
	~Texture();
	void init(VkImageUsageFlags usage, VkFormat format, VkSampleCountFlagBits samples, VkImageAspectFlags imageAspect);
	void setImageLayout(const VkImageLayout &newLayout, const uint32_t &mipLevels, const uint32_t &baseArrayLayer, const uint32_t &layerCount);
	void copyBufferToImage(const uint32_t &width, const uint32_t &height, const VkBuffer &buffer, const VkImage &image);
	void writeData(void *pixelData);
private:
	void createImage();
	Buffer* createStaging();
private:
	Image* mImage;
	VkImageCreateInfo mImageCI;
	IMAGE_DESC mDesc;
	uint32_t mWidth, mHeight;
	uint32_t mNumMipLevels;
	uint32_t mNumFaces;
};
#endif