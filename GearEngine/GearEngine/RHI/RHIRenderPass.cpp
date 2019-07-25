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

/**
*  使用示例:
*  LoadMaskBits load = LoadMaskBits::LOAD_ALL | LoadMaskBits::LOAD_DEPTH | LoadMaskBits::LOAD_STENCIL;
*/
VkRenderPass RHIRenderPass::createVariant(LoadMaskBits load, StoreMaskBits store, ClearMaskBits clear)
{
	//note:load操作和clear操作可能会发生冲突,但是这里不处理冲突,因为冲突应该由使用者自己负责
	std::vector<VkAttachmentDescription> attachmentDescs;
	std::vector<VkAttachmentReference> colorReferences;
	VkAttachmentReference depthReference;

	for (uint32_t i = 0; i < mDesc.numColorAttachments; i++)
	{
		VkAttachmentDescription attachmentDesc = {};
		attachmentDesc.flags = 0;
		attachmentDesc.format = mDesc.color[i].format;
		attachmentDesc.samples = getSampleFlags(mDesc.color[i].numSample);

		if (isInc(load, (LoadMaskBits)(1 << i)))
		{
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		}
		else if (isInc(clear, (ClearMaskBits)(1 << i)))
		{
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		}
		else
		{
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}

		if (isInc(store, (StoreMaskBits)(1 << i)))
		{
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		}
		else
		{
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
		attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentDescs.push_back(attachmentDesc);
	}
	if (mDesc.hasDepth) 
	{
		VkAttachmentDescription attachmentDesc = {};
		attachmentDesc.flags = 0;
		attachmentDesc.format = mDesc.depthStencil.format;
		attachmentDesc.samples = getSampleFlags(mDesc.depthStencil.numSample);

		if (isInc(load, LoadMaskBits::LOAD_DEPTH))
		{
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		}
		else if (isInc(clear, ClearMaskBits::CLEAR_DEPTH))
		{
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		}
		else
		{
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}

		if (isInc(store, StoreMaskBits::STORE_DEPTH))
		{
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		}
		else
		{
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}

		if (isInc(load, LoadMaskBits::LOAD_STENCIL))
		{
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		}
		else if (isInc(clear, ClearMaskBits::CLEAR_STENCIL))
		{
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		}
		else
		{
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}

		if (isInc(store, StoreMaskBits::STORE_STENCIL))
		{
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		}
		else
		{
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}

		attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachmentDescs.push_back(attachmentDesc);
	}

	return VkRenderPass();
}
