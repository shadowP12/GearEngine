#ifndef RHI_FRAME_BUFFER_H
#define RHI_FRAME_BUFFER_H
#include "RHIDefine.h"

class RHIDevice;
class RHIRenderPass;
class RHITextureView;
struct RHIFramebufferInfo
{
	RHIRenderPass* renderpass;
	RHITextureView* color[8];
	RHITextureView* depth;
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
private:
	RHIDevice* mDevice;
	RHIRenderPass* mRenderpass;
	VkFramebuffer mFramebuffer;
};
#endif