#include "Texture2D.h"
#include "RHI/RHI.h"
#include "RHI/RHIBuffers.h"
#include <assert.h>
Texture2D::Texture2D()
{
}

Texture2D::~Texture2D()
{
}

/*
void Texture::init(uint32_t w, uint32_t h, VkImageUsageFlags usage, VkFormat format, VkSampleCountFlagBits samples, VkImageAspectFlags imageAspect)
{
	mWidth = w;
	mHeight = h;
	mImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	mImageCI.pNext = nullptr;
	mImageCI.flags = 0;
	mImageCI.imageType = VK_IMAGE_TYPE_2D;
	mImageCI.usage = usage;
	//todo
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

	mImageCI.extent = {mWidth, mHeight, 1};
	mImageCI.mipLevels = mNumMipLevels;
	mImageCI.arrayLayers = mNumFaces;
	mImageCI.samples = samples;
	mImageCI.tiling = tiling;
	mImageCI.initialLayout = layout;
	mImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	mImageCI.queueFamilyIndexCount = 0;
	mImageCI.pQueueFamilyIndices = nullptr;
	mImageCI.format = format;

	mDesc.layout = layout;
	mDesc.format = format;
	mDesc.aspectFlags = imageAspect;
	mDesc.numFaces = mNumFaces;
	mDesc.numMipLevels = mNumMipLevels;
	mDesc.type = TextureType::TEX_TYPE_2D;
}

void Texture::setImageLayout(const VkFormat &format, const VkImageLayout &oldLayout, const VkImageLayout &newLayout)
{
	CommandBuffer commandBuffer = CommandBuffer();

	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = mDesc.image;
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
	{
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) 
		{
			imageMemoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else 
	{
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
	{
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
	{
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else 
	{
		throw std::invalid_argument("Unsupported texture layout transition!");
	}

	vkCmdPipelineBarrier(commandBuffer.getCommandBuffer(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

	commandBuffer.end();
	commandBuffer.submit();
}

void Texture::copyBufferToImage(const uint32_t & width, const uint32_t & height, const VkBuffer & buffer, const VkImage & image)
{
	CommandBuffer commandBuffer = CommandBuffer();
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {width,height,1};
	vkCmdCopyBufferToImage(commandBuffer.getCommandBuffer(), buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	commandBuffer.end();
	commandBuffer.submit();
}

void Texture::writeData(void * pixelData, VkDeviceSize size)
{
	Buffer* stagingBuffer = createStaging(size);
	stagingBuffer->writeData(0,size,pixelData);
	//todo:more
	setImageLayout(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(mWidth,mHeight,stagingBuffer->getBuffer(),mDesc.image);
	setImageLayout(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	stagingBuffer->destroy();
}

void Texture::createImage()
{
	VkImage image;

	if (vkCreateImage(VulkanContext::instance().getDevice(), &mImageCI, nullptr, &image))
	{
		throw std::runtime_error("failed to create image !");
	}
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(VulkanContext::instance().getDevice(), image, &memoryRequirements);

	VkMemoryPropertyFlags flags = (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);//todo

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = VulkanContext::instance().findMemoryType(memoryRequirements.memoryTypeBits, flags);

	VkDeviceMemory memory;
	if (vkAllocateMemory(VulkanContext::instance().getDevice(), &memoryAllocateInfo, nullptr, &memory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(VulkanContext::instance().getDevice(), image, memory, 0);
	mDesc.image = image;

	mImage = VulkanContext::instance().getResourceManager()->create<Image>(mDesc, memory);
}

Buffer * Texture2D::createStaging(uint32_t size)
{
	VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	return VulkanContext::instance().getResourceManager()->create<Buffer>(size, usageFlags, memoryPropertyFlags);
}

void Texture2D::createSampler(const VkFilter &filter, const VkSamplerAddressMode &addressMode)
{
	VkSamplerCreateInfo samplerCI = {};
	samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCI.magFilter = filter;
	samplerCI.minFilter = filter;
	samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCI.addressModeU = addressMode;
	samplerCI.addressModeV = addressMode;
	samplerCI.addressModeW = addressMode;
	samplerCI.mipLodBias = 0.0f;
	samplerCI.anisotropyEnable = static_cast<VkBool32>(false);//todo:add anisotropy control
	samplerCI.maxAnisotropy = 1;
	samplerCI.compareEnable = VK_FALSE;
	samplerCI.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCI.minLod = 0.0f;
	samplerCI.maxLod = static_cast<float>(mNumMipLevels);
	samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCI.unnormalizedCoordinates = VK_FALSE;
	if (vkCreateSampler(VulkanContext::instance().getDevice(), &samplerCI, nullptr, &mSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler !");
	}
}
*/
