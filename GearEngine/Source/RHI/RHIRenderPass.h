#ifndef RHI_RENDER_PASS_H
#define RHI_RENDER_PASS_H
#include "RHIDefine.h"
#include "Utility/Hash.h"
#include <unordered_map>
/**
  1.使用hash为索引去缓存vkrenderpass对象
  2.向外暴露参数和结构尽量不使用vulkan原生结构(由于快速实现暂时使用vulkan原生结构)
  3.暂不支持多个subpass
*/

class RHIDevice;

struct RHIRenderPassAttachmentInfo {
	VkFormat format = VK_FORMAT_UNDEFINED;
	uint32_t numSample = 1;
};

struct RHIRenderPassInfo {
	RHIRenderPassAttachmentInfo color[8];
	RHIRenderPassAttachmentInfo depthStencil;
	uint32_t numColorAttachments;
	bool hasDepth;
};

class RHIRenderPass
{
public:
	RHIRenderPass(RHIDevice* device, const RHIRenderPassInfo& desc);
	virtual ~RHIRenderPass();
	uint32_t getID() { return mID; }
	VkRenderPass getVkRenderPass(LoadMaskBits load, StoreMaskBits store, ClearMaskBits clear);
private:
	struct VariantKey
	{
		VariantKey(LoadMaskBits load, StoreMaskBits store, ClearMaskBits clear);

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
		StoreMaskBits storeMask;
		ClearMaskBits clearMask;
	};

	VkRenderPass createVariant(LoadMaskBits load, StoreMaskBits store, ClearMaskBits clear);
private:
	RHIDevice* mDevice;
	VkRenderPass mRenderPass;
	RHIRenderPassInfo mDesc;
	uint32_t mID;
	std::unordered_map<VariantKey, VkRenderPass, VariantKey::HashFunction, VariantKey::EqualFunction> mVariants;

	static uint32_t sNextValidID;
};
#endif
