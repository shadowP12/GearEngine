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
	uint32_t attachmentIdx = 0;

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

		VkAttachmentReference colorReference;
		colorReference.attachment = attachmentIdx;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorReferences.push_back(colorReference);

		attachmentIdx++;
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

		depthReference.attachment = attachmentIdx;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		attachmentIdx++;
	}

	VkSubpassDescription subpassDesc = {};
	subpassDesc.flags = 0;
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.colorAttachmentCount = mDesc.numColorAttachments;
	subpassDesc.inputAttachmentCount = 0;
	subpassDesc.pInputAttachments = nullptr;
	subpassDesc.preserveAttachmentCount = 0;
	subpassDesc.pPreserveAttachments = nullptr;
	subpassDesc.pResolveAttachments = nullptr;

	if (mDesc.numColorAttachments > 0)
	{
		subpassDesc.pColorAttachments = colorReferences.data();
	}
	else
	{
		subpassDesc.pColorAttachments = nullptr;
	}

	if (mDesc.hasDepth)
	{
		subpassDesc.pDepthStencilAttachment = &depthReference;
	}
	else
	{
		subpassDesc.pDepthStencilAttachment = nullptr;
	}

	//设置subpass之间的依赖(ps:暂不支持多个subpass)
	VkSubpassDependency dependencies[2];
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	dependencies[0].srcAccessMask = 0;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	dependencies[0].dependencyFlags = 0;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dstAccessMask = 0;
	dependencies[1].dependencyFlags = 0;

	//
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.flags = 0;
	renderPassInfo.attachmentCount = attachmentIdx;
	renderPassInfo.pAttachments = attachmentDescs.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDesc;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies;

	VkRenderPass ret;
	if (vkCreateRenderPass(mDevice->getDevice(), &renderPassInfo, nullptr, &ret) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
	return ret;
}
