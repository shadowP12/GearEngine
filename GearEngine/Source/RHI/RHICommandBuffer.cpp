#include "RHICommandBuffer.h"
#include "RHIDevice.h"
#include "RHIQueue.h"
#include "RHIBuffer.h"
#include "RHITexture.h"
#include "RHIRenderPass.h"
#include "RHIFramebuffer.h"
#include "RHIPipeline.h"
#include "RHISynchronization.h"
#include "RHIDescriptorSet.h"
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

	mFence = new RHIFence(mDevice);
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

void RHICommandBuffer::bindFramebuffer(RHIFramebuffer* fb)
{
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = fb->getRenderPass()->getHandle();
	renderPassInfo.framebuffer = fb->getHandle();
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent.width = fb->getWidth();
    renderPassInfo.renderArea.extent.height = fb->getHeight();

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 1.0f, 1.0f, 1.0, 0.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(mCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RHICommandBuffer::unbindFramebuffer()
{
	vkCmdEndRenderPass(mCommandBuffer);
}

void RHICommandBuffer::setViewport(int x, int y, int w, int h)
{
    VkViewport viewport = {};
    viewport.x = x;
    viewport.y = y;
    viewport.width = w;
    viewport.height = h;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
}

void RHICommandBuffer::setScissor(int x, int y, int w, int h)
{
    VkRect2D scissor = {};
    scissor.offset.x = x;
    scissor.offset.y = y;
    scissor.extent.width = w;
    scissor.extent.height = h;
    vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);
}

void RHICommandBuffer::bindVertexBuffer(RHIBuffer *vertexBuffer, uint32_t offset)
{
    VkBuffer vertexBuffers[] = { vertexBuffer->getHandle() };
    VkDeviceSize offsets[] = { offset };
    vkCmdBindVertexBuffers(mCommandBuffer, 0, 1, vertexBuffers, offsets);
}

void RHICommandBuffer::bindIndexBuffer(RHIBuffer *indexBuffer, uint32_t offset, VkIndexType type)
{
    vkCmdBindIndexBuffer(mCommandBuffer, indexBuffer->getHandle(), offset, type);
}

void RHICommandBuffer::bindGraphicsPipeline(RHIGraphicsPipeline *pipeline, RHIDescriptorSet** descriptorSets, uint32_t count)
{
    std::vector<VkDescriptorSet> sets;
    for (int i = 0; i < count; ++i)
    {
        sets.push_back(descriptorSets[i]->getHandle());
    }
    if(sets.size() > 0)
        vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
    vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getHandle());
}

void RHICommandBuffer::bindComputePipeline(RHIComputePipeline *pipeline, RHIDescriptorSet** descriptorSets, uint32_t count)
{
    std::vector<VkDescriptorSet> sets;
    for (int i = 0; i < count; ++i)
    {
        sets.push_back(descriptorSets[i]->getHandle());
    }
    if(sets.size() > 0)
        vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
    vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getHandle());
}

void RHICommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    vkCmdDraw(mCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void RHICommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	vkCmdDrawIndexed(mCommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void RHICommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    vkCmdDispatch(mCommandBuffer, groupCountX, groupCountY, groupCountZ);
}

void RHICommandBuffer::setResourceBarrier(uint32_t inBufferBarrierCount, RHIBufferBarrier* inBufferBarriers,
                                          uint32_t inTextureBarrierCount, RHITextureBarrier* inTextureBarriers)
{
    std::vector<VkImageMemoryBarrier> imageBarriers;
    std::vector<VkBufferMemoryBarrier> bufferBarriers;

    VkAccessFlags srcAccessFlags = 0;
    VkAccessFlags dstAccessFlags = 0;

    for (uint32_t i = 0; i < inBufferBarrierCount; ++i)
    {
        RHIBufferBarrier* trans = &inBufferBarriers[i];
        RHIBuffer* buffer = trans->buffer;
        bool flag = false;
        VkBufferMemoryBarrier bufferBarrier = {};
        if (!(trans->newState & buffer->getResourceState()))
        {
            flag = true;
            bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bufferBarrier.pNext = NULL;
            bufferBarrier.srcAccessMask = toVkAccessFlags(buffer->getResourceState());
            bufferBarrier.dstAccessMask = toVkAccessFlags(trans->newState);

            buffer->setResourceState(trans->newState);
        }
        else if(trans->newState == RESOURCE_STATE_UNORDERED_ACCESS)
        {
            flag = true;
            bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bufferBarrier.pNext = NULL;
            bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
        }

        if(flag)
        {
            bufferBarrier.buffer = buffer->getHandle();
            bufferBarrier.size = VK_WHOLE_SIZE;
            bufferBarrier.offset = 0;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            srcAccessFlags |= bufferBarrier.srcAccessMask;
            dstAccessFlags |= bufferBarrier.dstAccessMask;
            bufferBarriers.push_back(bufferBarrier);
        }
    }

    for (uint32_t i = 0; i < inTextureBarrierCount; ++i)
    {
        RHITextureBarrier* trans = &inTextureBarriers[i];
        RHITexture* texture = trans->texture;
        bool flag = false;
        VkImageMemoryBarrier imageBarrier = {};
        if (!(trans->newState & texture->getResourceState()))
        {
            flag = true;
            imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageBarrier.pNext = NULL;
            imageBarrier.srcAccessMask = toVkAccessFlags(texture->getResourceState());
            imageBarrier.dstAccessMask = toVkAccessFlags(trans->newState);
            imageBarrier.oldLayout = toVkImageLayout(texture->getResourceState());
            imageBarrier.newLayout = toVkImageLayout(trans->newState);

            texture->setResourceState(trans->newState);
        }
        else if(trans->newState == RESOURCE_STATE_UNORDERED_ACCESS)
        {
            flag = true;
            imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageBarrier.pNext = NULL;
            imageBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            imageBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
            imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        if(flag)
        {
            imageBarrier.image = texture->getHandle();
            imageBarrier.subresourceRange.aspectMask = texture->getAspectMask();
            imageBarrier.subresourceRange.baseMipLevel = 0;
            imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
            imageBarrier.subresourceRange.baseArrayLayer = 0;
            imageBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
            imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            srcAccessFlags |= imageBarrier.srcAccessMask;
            dstAccessFlags |= imageBarrier.dstAccessMask;
            imageBarriers.push_back(imageBarrier);
        }
    }
    VkPipelineStageFlags srcStageMask = toPipelineStageFlags(srcAccessFlags, mQueue->getType());
    VkPipelineStageFlags dstStageMask = toPipelineStageFlags(dstAccessFlags, mQueue->getType());

    if (imageBarriers.size() || bufferBarriers.size())
    {
        vkCmdPipelineBarrier(mCommandBuffer, srcStageMask, dstStageMask, 0, 0, NULL,
                             bufferBarriers.size(), bufferBarriers.data(),
                             imageBarriers.size(), imageBarriers.data());
    }
}

void RHICommandBuffer::updateBuffer(RHIBuffer *dstBuffer, uint32_t dstOffset, RHIBuffer *srcBuffer, uint32_t srcOffset, uint32_t size)
{
    VkBufferCopy copy = {};
    copy.srcOffset = srcOffset;
    copy.dstOffset = dstOffset;
    copy.size = size;
    vkCmdCopyBuffer(mCommandBuffer, srcBuffer->getHandle(), dstBuffer->getHandle(), 1, &copy);
}

void RHICommandBuffer::updateSubresource(RHITexture *dstTexture, RHIBuffer *srcBuffer, const SubresourceDataInfo &info)
{
    VkBufferImageCopy copy = {};
    copy.bufferOffset = 0;
    copy.bufferRowLength = 0;
    copy.bufferImageHeight = 0;
    copy.imageSubresource.aspectMask = dstTexture->getAspectMask();
    copy.imageSubresource.mipLevel = info.mipLevel;
    copy.imageSubresource.baseArrayLayer = info.arrayLayer;
    copy.imageSubresource.layerCount = 1;
    copy.imageOffset.x = 0;
    copy.imageOffset.y = 0;
    copy.imageOffset.z = 0;
    copy.imageExtent.width = dstTexture->getWidth();
    copy.imageExtent.height = dstTexture->getHeight();
    copy.imageExtent.depth = dstTexture->getDepth();

    vkCmdCopyBufferToImage(mCommandBuffer, srcBuffer->getHandle(), dstTexture->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
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