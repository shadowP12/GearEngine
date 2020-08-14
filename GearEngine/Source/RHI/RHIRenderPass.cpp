#include "RHIRenderPass.h"
#include "RHIDevice.h"

RHIRenderPass::RHIRenderPass(RHIDevice* device, const RHIRenderPassInfo& info)
	:mDevice(device)
{
	std::vector<VkAttachmentDescription> attachmentDescs;
	std::vector<VkAttachmentReference> colorReferences;
	VkAttachmentReference depthReference;
	uint32_t attachmentIdx = 0;

	for (uint32_t i = 0; i < info.numColorAttachments; i++)
	{
		VkAttachmentDescription attachmentDesc = {};
		attachmentDesc.flags = 0;
		attachmentDesc.format = info.color[i].format;
		attachmentDesc.samples = info.color[i].sampleCount;
		attachmentDesc.loadOp = info.color[i].loadOp;
		attachmentDesc.storeOp = info.color[i].storeOp;
		attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDesc.initialLayout = info.color[i].initialLayout;
		attachmentDesc.finalLayout = info.color[i].finalLayout;
		attachmentDescs.push_back(attachmentDesc);

		VkAttachmentReference colorReference;
		colorReference.attachment = attachmentIdx;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorReferences.push_back(colorReference);

		attachmentIdx++;
	}
	if (info.hasDepth)
	{
		VkAttachmentDescription attachmentDesc = {};
		attachmentDesc.flags = 0;
		attachmentDesc.format = info.depthStencil.format;
		attachmentDesc.samples = info.depthStencil.sampleCount;
		attachmentDesc.loadOp = info.depthStencil.depthLoadOp;
		attachmentDesc.storeOp = info.depthStencil.depthStoreOp;
		attachmentDesc.stencilLoadOp = info.depthStencil.stencilLoadOp;
		attachmentDesc.stencilStoreOp = info.depthStencil.stencilStoreOp;
		attachmentDesc.initialLayout = info.depthStencil.initialLayout;
		attachmentDesc.finalLayout = info.depthStencil.finalLayout;
		attachmentDescs.push_back(attachmentDesc);

		depthReference.attachment = attachmentIdx;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		attachmentIdx++;
	}

	VkSubpassDescription subpassDesc = {};
	subpassDesc.flags = 0;
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.colorAttachmentCount = info.numColorAttachments;
	subpassDesc.inputAttachmentCount = 0;
	subpassDesc.pInputAttachments = nullptr;
	subpassDesc.preserveAttachmentCount = 0;
	subpassDesc.pPreserveAttachments = nullptr;
	subpassDesc.pResolveAttachments = nullptr;

	if (info.numColorAttachments > 0)
	{
		subpassDesc.pColorAttachments = colorReferences.data();
	}
	else
	{
		subpassDesc.pColorAttachments = nullptr;
	}

	if (info.hasDepth)
	{
		subpassDesc.pDepthStencilAttachment = &depthReference;
	}
	else
	{
		subpassDesc.pDepthStencilAttachment = nullptr;
	}

	// todo
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

	CHECK_VKRESULT(vkCreateRenderPass(mDevice->getDevice(), &renderPassInfo, nullptr, &mRenderPass));
}

RHIRenderPass::~RHIRenderPass()
{
	vkDestroyRenderPass(mDevice->getDevice(), mRenderPass, nullptr);
}