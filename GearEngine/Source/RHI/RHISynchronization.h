#ifndef RHI_SYNCHRONIZATION_H
#define RHI_SYNCHRONIZATION_H
#include "RHIDefine.h"
class RHIDevice;
class RHIBuffer;
class RHITexture;

struct RHIBufferBarrier
{
    RHIBuffer* buffer;
    ResourceState  newState;
};

struct RHITextureBarrier
{
    RHITexture* texture;
    ResourceState newState;
};

class RHIFence
{
public:
    RHIFence(RHIDevice* device);
    ~RHIFence();
    void reset();
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
