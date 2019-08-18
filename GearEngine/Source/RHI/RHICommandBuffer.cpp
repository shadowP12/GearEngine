#include "RHICommandBuffer.h"
#include "RHIDevice.h"
#include "RHIQueue.h"

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
}

void RHICommandBuffer::end()
{
	//
	if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to end command buffer!");
	}
}