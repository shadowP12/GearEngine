#ifndef RHI_SYNCHRONIZATION_H
#define RHI_SYNCHRONIZATION_H
#include "RHIDefine.h"
class RHIDevice;

class RHIFence
{
public:
    RHIFence(RHIDevice* device);
    ~RHIFence();
    bool checkFenceState();
    VkFence getHandle() { return mFence; }
private:
    RHIDevice* mDevice;
    VkFence mFence;
};

class RHISemaphore
{
public:
    RHISemaphore(RHIDevice* device);
    ~RHISemaphore();
    VkSemaphore getHandle() { return mSemaphore; }
private:
    RHIDevice* mDevice;
    VkSemaphore mSemaphore;
};


#endif
