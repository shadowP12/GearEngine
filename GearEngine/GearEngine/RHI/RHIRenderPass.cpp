#include "RHIRenderPass.h"
#include "RHIDevice.h"

RHIRenderPass::VariantKey::VariantKey(LoadMaskBits load, StoreMaskBits store,
	ClearMaskBits clear)
	:loadMask(load), storeMask(store), clearMask(clear)
{ }

size_t RHIRenderPass::VariantKey::HashFunction::operator()(const VariantKey& v) const
{
	size_t hash = 0;
	Hash(hash, v.loadMask);
	Hash(hash, v.storeMask);
	Hash(hash, v.clearMask);

	return hash;
}

bool RHIRenderPass::VariantKey::EqualFunction::operator()(
	const VariantKey& lhs, const VariantKey& rhs) const
{
	return lhs.loadMask == rhs.loadMask && lhs.storeMask == rhs.storeMask && lhs.clearMask == rhs.clearMask;
}

RHIRenderPass::RHIRenderPass(RHIDevice* device, const RHIRenderPassDesc& desc)
	:mDevice(device), mDesc(desc)
{
}

RHIRenderPass::~RHIRenderPass()
{

}

VkRenderPass RHIRenderPass::getVkRenderPass(LoadMaskBits load, StoreMaskBits store, ClearMaskBits clear)
{
	VariantKey key(load, store, clear);
	auto iterFind = mVariants.find(key);
	if (iterFind != mVariants.end())
		return iterFind->second;

	VkRenderPass newVariant = createVariant(load, store, clear);
	mVariants[key] = newVariant;

	return newVariant;
}

VkRenderPass RHIRenderPass::createVariant(LoadMaskBits load, StoreMaskBits store, ClearMaskBits clear)
{
	std::vector<VkAttachmentDescription> attachmentDescs;
	for (uint32_t i = 0; i < mNumColorAttachments; i++)
	{
		VkAttachmentDescription attachmentDesc = {};
		
	}
	return VkRenderPass();
}
