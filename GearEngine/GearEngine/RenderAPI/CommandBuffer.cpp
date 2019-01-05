#include "CommandBuffer.h"
#include "VulkanContext.h"

CommandBuffer::CommandBuffer(bool isBegin, VkQueueFlagBits queueType, VkCommandBufferLevel bufferLevel) :
	mQueueType(queueType),
	mBufferLevel(bufferLevel),
	mCommandBuffer(VK_NULL_HANDLE),
	mRunning(false)
{
	auto logicalDevice = VulkanContext::instance().getDevice();
	auto commandPool = VulkanContext::instance().getCommandPool();

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = bufferLevel;
	commandBufferAllocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(logicalDevice, &commandBufferAllocateInfo, &mCommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffer!");
	}

	if (isBegin)
	{
		begin();
	}
}

CommandBuffer::~CommandBuffer()
{
	auto logicalDevice = VulkanContext::instance().getDevice();
	auto commandPool = VulkanContext::instance().getCommandPool();

	vkFreeCommandBuffers(logicalDevice, commandPool, 1, &mCommandBuffer);
}

void CommandBuffer::begin(VkCommandBufferUsageFlags usage)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = usage;
	if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin command buffer!");
	}
	mRunning = true;
}

void CommandBuffer::end()
{
	if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to end command buffer!");
	}

	mRunning = false;
}

void CommandBuffer::submit(VkSemaphore signalSemaphore, VkFence fence, bool createFence)
{
	auto logicalDevice = VulkanContext::instance().getDevice();
	auto queueSelected = getQueue();

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffer;

	if (signalSemaphore != VK_NULL_HANDLE)
	{
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore;
	}

	bool createdFence = false;

	if (fence == VK_NULL_HANDLE && createFence)
	{
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = 0;
		if (vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create fence!");
		}

		createdFence = true;
	}

	if (fence != VK_NULL_HANDLE)
	{
		if (vkResetFences(logicalDevice, 1, &fence) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to reset fence!");
		}
	}
	if (vkQueueSubmit(queueSelected, 1, &submitInfo, fence) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit !");
	}

	if (fence != VK_NULL_HANDLE)
	{
		vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

		if (createdFence)
		{
			vkDestroyFence(logicalDevice, fence, nullptr);
		}
	}
}

VkQueue CommandBuffer::getQueue()
{
	switch (mQueueType)
	{
	case VK_QUEUE_GRAPHICS_BIT:
		return VulkanContext::instance().getGraphicsQueue();
	case VK_QUEUE_COMPUTE_BIT:
		return VulkanContext::instance().getComputeQueue();
	default:
		return nullptr;
	}
}