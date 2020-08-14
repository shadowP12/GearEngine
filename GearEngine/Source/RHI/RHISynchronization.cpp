#include "RHISynchronization.h"
#include "RHIDevice.h"
#include <vector>

RHIFence::RHIFence(RHIDevice* device)
    :mDevice(device)
{
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    CHECK_VKRESULT(vkCreateFence(mDevice->getDevice(), &fenceInfo, nullptr, &mFence));
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
    CHECK_VKRESULT(vkCreateSemaphore(mDevice->getDevice(), &semaphoreInfo, nullptr, &mSemaphore));
}

RHISemaphore::~RHISemaphore()
{
    vkDestroySemaphore(mDevice->getDevice(), mSemaphore, nullptr);
}