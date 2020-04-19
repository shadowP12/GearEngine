#ifndef RHI_CONTEXT_H
#define RHI_CONTEXT_H
#include <vector>
class RHIDevice;
class RHICommandBuffer;
class RHICommandBufferPool;
class RHIContext
{
public:
    RHIContext(RHIDevice* device);
    ~RHIContext();
    RHIDevice* getDevice(){return mDevice;}
    RHICommandBufferPool* getCmdBufferPool(){return mCmdBufferPool;}
    RHICommandBuffer* getActiveCmdBuffer();
private:
    RHIDevice* mDevice;
    RHICommandBufferPool* mCmdBufferPool;
};


#endif
