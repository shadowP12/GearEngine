#include "Renderer.h"

void Renderer::draw()
{
	VkResult result = vkAcquireNextImageKHR(VulkanContext::instance().getDevice(), mSwapChain->getSwapchain(), std::numeric_limits<uint64_t>::max(), mImageAvailableSemaphore, VK_NULL_HANDLE, &mCurFrameIndex);

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	//build draw command
	CommandBuffer drawCommand(false);
	drawCommand.begin();
	buildCommandBuffer(drawCommand);
	drawCommand.end();

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { mImageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	VkCommandBuffer commandBufferr = drawCommand.getCommandBuffer();

	submitInfo.pCommandBuffers = &commandBufferr;

	VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(VulkanContext::instance().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { mSwapChain->getSwapchain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &mCurFrameIndex;

	result = vkQueuePresentKHR(VulkanContext::instance().getPresentQueue(), &presentInfo);

	if (result != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	vkQueueWaitIdle(VulkanContext::instance().getPresentQueue());
}

void Renderer::buildCommandBuffer(CommandBuffer& commandBuffer)
{
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = mMainRenderPass->getRenderPass();
	renderPassInfo.framebuffer = mSwapChain->getFrameBuffer(mCurFrameIndex);
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { mWidth,mHeight };

	//remove
	std::vector<VkClearValue> clearValues;
	VkClearValue clear1;
	clear1.color = { 1.0f, 0.0f, 0.0f, 1.0f };
	clearValues.push_back(clear1);

	VkClearValue clear2;
	clear2.depthStencil = { 1.0f, 0 };
	clearValues.push_back(clear2);

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer.getCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = mWidth;
	viewport.height = mHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer.getCommandBuffer(), 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = { mWidth,mHeight };
	vkCmdSetScissor(commandBuffer.getCommandBuffer(), 0, 1, &scissor);

	vkCmdEndRenderPass(commandBuffer.getCommandBuffer());
}

void Renderer::createSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(VulkanContext::instance().getDevice(), &semaphoreInfo, nullptr, &mImageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(VulkanContext::instance().getDevice(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphore) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create semaphores!");
	}
}
