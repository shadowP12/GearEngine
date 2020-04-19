#include "RHIContext.h"
#include "RHIDevice.h"
#include "RHIBuffers.h"
#include "RHICommandBuffer.h"
#include "RHIQueue.h"
#include "RHIProgram.h"
#include "RHIRenderPass.h"
#include "RHIFramebuffer.h"
#include "RHITexture.h"
#include "RHITextureView.h"

RHIContext::RHIContext(RHIDevice* device)
    :mDevice(device)
{
    mCmdBufferPool = mDevice->createCmdBufferPool();
}

RHIContext::~RHIContext()
{
    SAFE_DELETE(mCmdBufferPool);
}

RHICommandBuffer* RHIContext::getActiveCmdBuffer()
{
    return mCmdBufferPool->getActiveCmdBuffer();
}
