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
	UINT32 width = 0;
	UINT32 height = 0;
	UINT32 layers = 0;
};

class RHIFramebuffer
{
public:
	RHIFramebuffer(RHIDevice* device, const RHIFramebufferInfo& info);
	~RHIFramebuffer();

private:
	RHIDevice* mDevice;
	RHIRenderPass* mRenderpass;
	VkFramebuffer mFramebuffer;
};
#endif