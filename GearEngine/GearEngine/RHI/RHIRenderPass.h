#ifndef RHI_RENDER_PASS_H
#define RHI_RENDER_PASS_H
#include "RHIDefine.h"
#include "Utility/Hash.h"
#include <unordered_map>
/**
  使用hash为索引去缓存vkrenderpass对象
*/

class RHIDevice;

struct RHIColorAttachment {

};

struct RHIDepthStencilAttachment {

};

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
	struct VariantKey
	{
		VariantKey(LoadMaskBits loadMask, ReadMaskBits readMask, ClearMaskBits clearMask);

		class HashFunction
		{
		public:
			size_t operator()(const VariantKey& key) const;
		};

		class EqualFunction
		{
		public:
			bool operator()(const VariantKey& lhs, const VariantKey& rhs) const;
		};

		LoadMaskBits loadMask;
		ReadMaskBits readMask;
		ClearMaskBits clearMask;
	};
private:
	VkRenderPass mRenderPass;
	std::unordered_map<VariantKey, VkRenderPass, VariantKey::HashFunction, VariantKey::EqualFunction> mVariants;
};
#endif
