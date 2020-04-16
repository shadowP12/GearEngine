#ifndef RHI_CONTEXT_H
#define RHI_CONTEXT_H
#include <vector>
class RHIDevice;
class RHICommandBuffer;
class RHICommandBufferPool;
class RHIProgram;
class RHIVertexBuffer;
class RHIUniformBuffer;
class RHIIndexBuffer;
class RHITransferBuffer;
class RHIRenderPass;
class RHIFramebuffer;
class RHITexture;
class RHITextureView;

struct RHIProgramInfo;
struct RHIRenderPassInfo;
struct RHIFramebufferInfo;
struct RHITextureInfo;
struct RHITextureViewInfo;
class RHIContext
{
public:
    RHIContext(RHIDevice* device);
    ~RHIContext();
    RHIProgram* createProgram(const RHIProgramInfo& programInfo);
    RHIRenderPass* createRenderPass(const RHIRenderPassInfo& renderPassInfo);
    RHIFramebuffer* createFramebuffer(const RHIFramebufferInfo& framebufferInfo);
    RHITexture* createTexture(const RHITextureInfo& textureInfo);
    RHITextureView* createTextureView(const RHITextureViewInfo& viewInfo);
    RHIUniformBuffer* createUniformBuffer(uint32_t size);
    RHIVertexBuffer* createVertexBuffer(uint32_t elementSize, uint32_t vertexCount);
    RHIIndexBuffer* createIndexBuffer(uint32_t elementSize, uint32_t indexCount);
    RHITransferBuffer* createTransferBuffer(uint32_t size);
    
private:
    RHIDevice* mDevice;
    RHICommandBufferPool* mCommandPool;
};


#endif
