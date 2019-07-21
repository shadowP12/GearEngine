#ifndef RHI_RENDER_PASS_H
#define RHI_RENDER_PASS_H
#include "RHIDefine.h"
#include "Utility/Hash.h"
#include <unordered_map>
/**
  使用hash为索引去缓存vkrenderpass对象
*/

class RHIDevice;

class RHIRenderTargetLayout
{
public:
	RHIRenderTargetLayout(RHIDevice* device);
	virtual ~RHIRenderTargetLayout();
private:
	RHIDevice* mDevice;
};

class RHIRenderPass 
{
public:
	RHIRenderPass(RHIDevice* device, const RHIRenderTargetLayout& layout);
	virtual ~RHIRenderPass();
private:
	VkRenderPass mRenderPass;
};
#endif
