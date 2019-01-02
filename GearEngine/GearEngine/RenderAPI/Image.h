#ifndef IMAGE_H
#define IMAGE_H
#include "VulkanResource.h"
enum class TextureType
{
	TEX_TYPE_1D,
	TEX_TYPE_2D,
	TEX_TYPE_3D,
	TEX_TYPE_CUBE_MAP
};
struct VULKAN_IMAGE_DESC
{
	VkImage image;
	VkImageLayout layout;
	VkImageAspectFlags  aspectFlags;
	TextureType type;
	VkFormat format;
	uint32_t numFaces;
	uint32_t numMipLevels;
};
class Image : public VulkanResource
{
public:
	Image(VulkanResourceManager* manager, VULKAN_IMAGE_DESC desc);
	~Image();
private:
	void createView(VkImageAspectFlags aspectMask);
private:
	VkImage mImage;
	VkImageView mImageView;
	uint32_t mNumFaces;
	uint32_t mNumMipLevels;
	VkImageViewCreateInfo mImageViewCI;
};

#endif