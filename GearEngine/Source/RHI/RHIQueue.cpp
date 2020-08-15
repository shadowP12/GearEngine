#include "RHIQueue.h"
#include "RHIDevice.h"
#include "RHICommandBuffer.h"
#include "RHISwapChain.h"
#include "RHISynchronization.h"

RHIQueue::RHIQueue(RHIDevice* device, VkQueue queue, uint32_t familyIndex, QueueType type)
	:mDevice(device), mQueue(queue), mFamilyIndex(familyIndex), mType(type)
{
}

RHIQueue::~RHIQueue()
{
}

void RHIQueue::submit(const RHIQueueSubmitInfo &info)
{
    std::vector<VkPipelineStageFlags> waitStages;
    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkSemaphore> signalSemaphores;

    for(int i = 0; i < info.waitSemaphoreCount; i++)
    {
        waitSemaphores.push_back(info.waitSemaphores[i]->getHandle());
        waitStages.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    }

    for(int i = 0; i < info.signalSemaphoreCount; i++)
    {
        signalSemaphores.push_back(info.signalSemaphores[i]->getHandle());
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores = waitSemaphores.data();

    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.signalSemaphoreCount = signalSemaphores.size();
    submitInfo.pSignalSemaphores = signalSemaphores.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &info.cmdBuf->mCommandBuffer;
    info.cmdBuf->mFence->reset();
    CHECK_VKRESULT(vkQueueSubmit(mQueue, 1, &submitInfo, info.cmdBuf->mFence->getHandle()));

    info.cmdBuf->mState = RHICommandBuffer::State::Submitted;
}

void RHIQueue::Present(const RHIQueuePresentInfo &info)
{
    std::vector<VkSemaphore> waitSemaphores;
    for(int i = 0; i < info.waitSemaphoreCount; i++)
    {
        waitSemaphores.push_back(info.waitSemaphores[i]->getHandle());
    }
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = waitSemaphores.size();
    presentInfo.pWaitSemaphores = waitSemaphores.data();
    VkSwapchainKHR swapChains[] = { info.swapChain->getHandle() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &info.index;

    CHECK_VKRESULT(vkQueuePresentKHR(mQueue, &presentInfo));
    CHECK_VKRESULT(vkQueueWaitIdle(mQueue));
}

void RHIQueue::waitIdle()
{
    vkQueueWaitIdle(mQueue);
}