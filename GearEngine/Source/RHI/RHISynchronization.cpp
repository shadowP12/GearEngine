#include "RHISynchronization.h"
#include "RHIDevice.h"
RHIFence::RHIFence(RHIDevice* device)
    :mDevice(device)
{
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(mDevice->getDevice(), &fenceInfo, nullptr, &mFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create fence!");
    }
}

RHIFence::~RHIFence()
{
    vkDestroyFence(mDevice->getDevice(), mFence, nullptr);
}

void RHIFence::reset()
{
    vkResetFences(mDevice->getDevice(), 1, &mFence);
}

bool RHIFence::checkFenceState()
{
    VkResult result = vkGetFenceStatus(mDevice->getDevice(), mFence);
    if(result == VK_SUCCESS)
    {
        return true;
    }
    return false;
}

RHISemaphore::RHISemaphore(RHIDevice* device)
    :mDevice(device)
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(mDevice->getDevice(), &semaphoreInfo, nullptr, &mSemaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create semaphores!");
    }
}

RHISemaphore::~RHISemaphore()
{
    vkDestroySemaphore(mDevice->getDevice(), mSemaphore, nullptr);
}