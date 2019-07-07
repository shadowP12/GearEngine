#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H
#include "VulkanContext.h"
class  CommandBuffer
{
public:
	explicit CommandBuffer(bool isBegin = true, VkQueueFlagBits queueType = VK_QUEUE_GRAPHICS_BIT, VkCommandBufferLevel bufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	~CommandBuffer();

	void begin(VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	void end();

	void submit(VkSemaphore signalSemaphore = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE, bool createFence = true);

	bool isRunning() { return mRunning; }

	VkCommandBuffer getCommandBuffer() { return mCommandBuffer; }
private:
	VkQueue getQueue();
private:
	VkQueueFlagBits mQueueType;
	VkCommandBufferLevel mBufferLevel;
	VkCommandBuffer mCommandBuffer;
	bool mRunning;
};
#endif