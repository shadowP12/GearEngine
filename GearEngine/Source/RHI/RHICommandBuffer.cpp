#include "RHICommandBuffer.h"
#include "RHIDevice.h"
#include "RHIQueue.h"
#include "RHIRenderPass.h"
#include "RHIFramebuffer.h"
#include "RHIPipelineState.h"
#include "Utility/Log.h"
#include <array>
RHICommandBufferPool::RHICommandBufferPool(RHIDevice* device, RHIQueue* queue, bool reset)
	:mDevice(device), mQueue(queue)
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = mQueue->getFamilyIndex();
	if (reset)
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	else
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	if (vkCreateCommandPool(mDevice->getDevice(), &poolInfo, nullptr, &mPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

RHICommandBufferPool::~RHICommandBufferPool()
{
	vkDestroyCommandPool(mDevice->getDevice(), mPool, nullptr);
}

RHICommandBuffer* RHICommandBufferPool::allocCommandBuffer(bool primary)
{
	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = mPool;
	if (primary)
	{
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	}
	else
	{
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	}
	allocateInfo.commandBufferCount = 1;
	
	RHICommandBuffer* commandBuffer = new RHICommandBuffer(mDevice, mQueue, this);
	if (vkAllocateCommandBuffers(mDevice->getDevice(), &allocateInfo, &commandBuffer->mCommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffer!");
	}
	return commandBuffer;
}

void RHICommandBufferPool::freeCommandBuffer(RHICommandBuffer * cmd)
{
	SAFE_DELETE(cmd);
}

RHICommandBuffer::RHICommandBuffer(RHIDevice* device, RHIQueue* queue, RHICommandBufferPool* pool)
	:mDevice(device), mQueue(queue), mCommandPool(pool)
{
	//分配fence
	//todo:后续将创建fence步骤转移至device上
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	
	if (vkCreateFence(mDevice->getDevice(), &fenceInfo, nullptr, &mFence) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create fence!");
	}
}

RHICommandBuffer::~RHICommandBuffer()
{
	vkDestroyFence(mDevice->getDevice(), mFence, nullptr);
	vkFreeCommandBuffers(mDevice->getDevice(), mCommandPool->mPool, 1, &mCommandBuffer);
}

void RHICommandBuffer::begin()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//这里设置成每次提交后都会自动重置命令
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin command buffer!");
	}
	mState = State::Recording;
}

void RHICommandBuffer::end()
{
	//
	if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to end command buffer!");
	}
	mState = State::RecordingDone;
}

void RHICommandBuffer::beginRenderPass(glm::vec4 renderArea)
{
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = mFramebuffer->getRenderPass()->getVkRenderPass(LoadMaskBits::LOAD_NONE, StoreMaskBits::STORE_NONE, ClearMaskBits::CLEAR_NONE);
	renderPassInfo.framebuffer = mFramebuffer->getHandle();
	renderPassInfo.renderArea.offset = {renderArea.x, renderArea.y};
	VkExtent2D extent;
	extent.width = renderArea.z;
	extent.height = renderArea.w;
	renderPassInfo.renderArea.extent = extent;

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(mCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	mState = State::RecordingRenderPass;
}

void RHICommandBuffer::endRenderPass()
{
	vkCmdEndRenderPass(mCommandBuffer);
	mState = State::Recording;
}

void RHICommandBuffer::setRenderTarget(RHIFramebuffer* framebuffer)
{
	// todo: 添加判断是否处于renderpass中
	mFramebuffer = framebuffer;
}

void RHICommandBuffer::bindGraphicsPipelineState(RHIGraphicsPipelineState* pipelineState)
{
	mGraphicsPipelineState = pipelineState;
}

void RHICommandBuffer::setViewport(glm::vec4 viewport)
{
	mViewport = viewport;
}

void RHICommandBuffer::setScissor(glm::vec4 scissor)
{
	mScissor = scissor;
}

void RHICommandBuffer::bindVertexBuffer(RHIVertexBuffer* vertexBuffer)
{
	mVertexBuffer = vertexBuffer;
}

void RHICommandBuffer::bindIndexBuffer(RHIIndexBuffer* indexBuffer)
{
	mIndexBuffer = indexBuffer;
}

void RHICommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	// todo: 还需添加更多状态判断,如是否处于renderpass中或是否处于读写状态等
	if (mFramebuffer == nullptr) 
	{
		LOGE("invalid framebuffer");
		return;
	}
	if (mGraphicsPipelineState == nullptr)
	{
		LOGE("invalid GraphicsPipelineState");
		return;
	}
		
	if (mVertexBuffer == nullptr)
	{
		LOGE("invalid VertexBuffer");
		return;
	}
		
	if (mIndexBuffer == nullptr)
	{
		LOGE("invalid IndexBuffer");
		return;
	}
	VkViewport viewport = {};
	viewport.x = mViewport.x;
	viewport.y = mViewport.y;
	viewport.width = mViewport.z;
	viewport.height = mViewport.w;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset = { mScissor.x, mScissor.y };
	VkExtent2D extent;
	extent.width = mScissor.z;
	extent.height = mScissor.w;
	scissor.extent = extent;
	vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipelineState->getPipeline(mFramebuffer->getRenderPass()));

	VkBuffer vertexBuffers[] = { mVertexBuffer->getBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(mCommandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(mCommandBuffer, mIndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	std::vector<VkDescriptorSet> sets = mGraphicsPipelineState->getDescSets();
	vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipelineState->getLayout(), 0, sets.size(), sets.data(), 0, nullptr);

	vkCmdDrawIndexed(mCommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}