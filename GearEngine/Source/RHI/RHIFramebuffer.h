#ifndef RHI_FRAME_BUFFER_H
#define RHI_FRAME_BUFFER_H
#include "RHIDefine.h"

class RHIDevice;
class RHIRenderPass;
class RHITextureView;
class RHITexture;

struct RHIFramebufferInfo
{
	RHIRenderPass* renderpass;
	RHITexture* color[8];
	RHITexture* depth;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t layers = 0;
	uint32_t numColorAttachments;
	bool hasDepth;
};

class RHIFramebuffer
{
public:
	RHIFramebuffer(RHIDevice* device, const RHIFramebufferInfo& info);
	~RHIFramebuffer();
	RHIRenderPass* getRenderPass() { return mRenderpass; }
	VkFramebuffer getHandle() { return mFramebuffer; }
	uint32_t getWidth() { return mWidth; }
	uint32_t getHeight() { return mHeight; }
private:
	RHIDevice* mDevice;
	RHIRenderPass* mRenderpass;
	VkFramebuffer mFramebuffer;
	uint32_t mWidth;
	uint32_t mHeight;
};
#endif