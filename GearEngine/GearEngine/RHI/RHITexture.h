#ifndef RHI_TEXTURE_H
#define RHI_TEXTURE_H

enum class TextureType
{
	TEX_TYPE_1D,
	TEX_TYPE_2D,
	TEX_TYPE_3D,
	TEX_TYPE_CUBE_MAP
};
struct IMAGE_DESC
{
	VkImage image;
	VkImageLayout layout;
	VkImageAspectFlags  aspectFlags;
	TextureType type;
	VkFormat format;
	uint32_t numFaces;
	uint32_t numMipLevels;
};

class RHIDevice;

class RHITexture
{
public:
	RHITexture(RHIDevice* device);
	virtual~RHITexture();
	VkImage getImage() { return mImage; }
	VkImageView getView() { return mImageView; }
private:
private:
	RHIDevice* mDevice;
	VkImage mImage;
	VkImageView mImageView;
	uint32_t mNumFaces;
	uint32_t mNumMipLevels;
	VkImageViewCreateInfo mImageViewCI;
	VkDeviceMemory mBufferMemory;
};

#endif