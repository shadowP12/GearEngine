#ifndef RHI_TEXTURE_H
#define RHI_TEXTURE_H
#include "RHIDefine.h"

struct RHITextureInfo
{
	VkImageType type;
	VkFormat format;
	VkExtent3D extent;
	uint32_t mipLevels; 
	uint32_t arrayLayers;
	VkSampleCountFlagBits samples;
	VkImageUsageFlags usage;
};

class RHIDevice;

class RHITexture
{
public:
	RHITexture(RHIDevice* device, const RHITextureInfo& textureDesc);
	virtual~RHITexture();
	VkImage getHandle() { return mImage; }
private:
private:
	friend class RHITextureView;
	RHIDevice* mDevice;
	VkImage mImage;
	VkDeviceMemory mMemory;
};

#endif