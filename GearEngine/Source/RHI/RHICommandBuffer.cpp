#include "RHICommandBuffer.h"
#include "RHIDevice.h"
#include "RHIQueue.h"
#include "RHIRenderPass.h"
#include "RHIFramebuffer.h"
#include "RHIPipelineState.h"
#include "RHISynchronization.h"
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
    for(int i = 0; i < mCmdBuffers.size(); i++)
    {
        SAFE_DELETE(mCmdBuffers[i]);
    }
	vkDestroyCommandPool(mDevice->getDevice(), mPool, nullptr);
}

RHICommandBuffer* RHICommandBufferPool::getActiveCmdBuffer()
{
    RHICommandBuffer* cmdBuffer = nullptr;
    for (int i = 0; i < mCmdBuffers.size(); ++i)
    {
        mCmdBuffers[i]->refreshFenceStatus();
        if(mCmdBuffers[i]->mState == RHICommandBuffer::State::Ready)
        {
            return mCmdBuffers[i];
        }
    }
    cmdBuffer = new RHICommandBuffer(mDevice, mQueue, this);
    mCmdBuffers.push_back(cmdBuffer);
    return cmdBuffer;
}

RHICommandBuffer::RHICommandBuffer(RHIDevice* device, RHIQueue* queue, RHICommandBufferPool* pool)
	:mDevice(device), mQueue(queue), mCommandPool(pool)
{
	mCommandBuffer = nullptr;
	mFramebuffer = nullptr;
	mGraphicsPipelineState = nullptr;
	mIndexBuffer = nullptr;
	mViewport = glm::vec4(0.0f);
	mScissor = glm::vec4(0.0f);
	mState = State::Ready;

    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = mCommandPool->getHandle();
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(mDevice->getDevice(), &allocateInfo, &mCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffer!");
    }

	mFence = mDevice->createFence();
}

RHICommandBuffer::~RHICommandBuffer()
{
    SAFE_DELETE(mFence);
	vkFreeCommandBuffers(mDevice->getDevice(), mCommandPool->mPool, 1, &mCommandBuffer);
}

void RHICommandBuffer::begin()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT ;
	if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin command buffer!");
	}
	mState = State::Recording;
}

void RHICommandBuffer::end()
{
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
	renderPassInfo.renderPass = mFramebuffer->getRenderPass()->getHandle();
	renderPassInfo.framebuffer = mFramebuffer->getHandle();
	renderPassInfo.renderArea.offset.x = (int32_t)renderArea.x;
	renderPassInfo.renderArea.offset.y = (int32_t)renderArea.y;
	VkExtent2D extent;
	extent.width = (int32_t)renderArea.z;
	extent.height = (int32_t)renderArea.w;
	renderPassInfo.renderArea.extent = extent;

	std::array<VkClearValue, 1> clearValues = {};
	clearValues[0].color = { 1.0f, 1.0f, 0.0, 0.0f };
	//clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(mCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RHICommandBuffer::endRenderPass()
{
	vkCmdEndRenderPass(mCommandBuffer);
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
	scissor.offset.x = (int32_t)mScissor.x;
	scissor.offset.y = (int32_t)mScissor.y;
	VkExtent2D extent;
	extent.width = (int32_t)mScissor.z;
	extent.height = (int32_t)mScissor.w;
	scissor.extent = extent;
	vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipelineState->getPipeline(mFramebuffer->getRenderPass()));

	VkBuffer vertexBuffers[] = { mVertexBuffer->getBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(mCommandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(mCommandBuffer, mIndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	std::vector<VkDescriptorSet> sets = mGraphicsPipelineState->getDescSets();
	if (sets.size() > 0)
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipelineState->getLayout(), 0, sets.size(), sets.data(), 0, nullptr);

	vkCmdDrawIndexed(mCommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void RHICommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
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
	scissor.offset.x = (int32_t)mScissor.x;
	scissor.offset.y = (int32_t)mScissor.y;
	VkExtent2D extent;
	extent.width = (int32_t)mScissor.z;
	extent.height = (int32_t)mScissor.w;
	scissor.extent = extent;
	vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipelineState->getPipeline(mFramebuffer->getRenderPass()));

	VkBuffer vertexBuffers[] = { mVertexBuffer->getBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(mCommandBuffer, 0, 1, vertexBuffers, offsets);
	std::vector<VkDescriptorSet> sets = mGraphicsPipelineState->getDescSets();
	if(sets.size() > 0)
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipelineState->getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
	vkCmdDraw(mCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void RHICommandBuffer::addWaitSemaphore(RHISemaphore* semaphore)
{
    mWaitSemaphores.push_back(semaphore);
}

void RHICommandBuffer::addSignalSemaphore(RHISemaphore* semaphore)
{
    mSignalSemaphores.push_back(semaphore);
}

void RHICommandBuffer::submit()
{
    std::vector<VkPipelineStageFlags> waitStages;
    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkSemaphore> signalSemaphores;

    for(int i = 0; i < mWaitSemaphores.size(); i++)
    {
        waitSemaphores.push_back(mWaitSemaphores[i]->getHandle());
        waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }

    for(int i = 0; i < mSignalSemaphores.size(); i++)
    {
        signalSemaphores.push_back(mSignalSemaphores[i]->getHandle());
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores = waitSemaphores.data();

    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.signalSemaphoreCount = signalSemaphores.size();
    submitInfo.pSignalSemaphores = signalSemaphores.data();
    submitInfo.commandBufferCount = 1;
    VkCommandBuffer cmd[] = { mCommandBuffer };
    submitInfo.pCommandBuffers = cmd;

    mFence->reset();

    if (vkQueueSubmit(mDevice->getGraphicsQueue()->getHandle(), 1, &submitInfo, mFence->getHandle()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit present command buffer!");
    }

    mWaitSemaphores.clear();
    mSignalSemaphores.clear();
    mState = State::Submitted;
}

void RHICommandBuffer::refreshFenceStatus()
{
    if(!mFence->checkFenceState())
    {
        return;
    }
    vkResetCommandBuffer(mCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    mState = State::Ready;
}