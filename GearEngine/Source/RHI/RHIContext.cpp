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
}

RHIContext::~RHIContext()
{
}

RHIUniformBuffer* RHIContext::createUniformBuffer(uint32_t size)
{
    RHIUniformBuffer* ret = new RHIUniformBuffer(mDevice, size);
    return ret;
}

RHIVertexBuffer* RHIContext::createVertexBuffer(uint32_t elementSize, uint32_t vertexCount)
{
    RHIVertexBuffer* ret = new RHIVertexBuffer(mDevice, elementSize, vertexCount);
    return ret;
}

RHIIndexBuffer* RHIContext::createIndexBuffer(uint32_t elementSize, uint32_t indexCount)
{
    RHIIndexBuffer* ret = new RHIIndexBuffer(mDevice, elementSize, indexCount);
    return ret;
}

RHITransferBuffer* RHIContext::createTransferBuffer(uint32_t size)
{
    RHITransferBuffer* ret = new RHITransferBuffer(mDevice, size);
    return ret;
}

RHIRenderPass* RHIContext::createRenderPass(const RHIRenderPassInfo& renderPassInfo)
{
    RHIRenderPass* ret = new RHIRenderPass(mDevice, renderPassInfo);
    return ret;
}

RHIFramebuffer* RHIContext::createFramebuffer(const RHIFramebufferInfo& framebufferInfo)
{
    RHIFramebuffer* ret = new RHIFramebuffer(mDevice, framebufferInfo);
    return ret;
}

RHIProgram * RHIContext::createProgram(const RHIProgramInfo & programInfo)
{
    RHIProgram* ret = new RHIProgram(mDevice, programInfo);
    return ret;
}

RHITexture* RHIContext::createTexture(const RHITextureInfo& textureInfo)
{
    RHITexture* ret = new RHITexture(mDevice, textureInfo);
    return ret;
}

RHITextureView* RHIContext::createTextureView(const RHITextureViewInfo& viewInfo)
{
    RHITextureView* ret = new RHITextureView(mDevice, viewInfo);
    return ret;
}
