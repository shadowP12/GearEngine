#pragma once
#include "Core/GearDefine.h"
#include <functional>

namespace Blast {
    class ShaderCompiler;
    class GfxContext;
    class GfxSurface;
    class GfxSwapchain;
    class GfxRenderPass;
    class GfxFramebuffer;
    class GfxQueue;
    class GfxFence;
    class GfxSemaphore;
    class GfxSemaphore;
    class GfxCommandBufferPool;
    class GfxCommandBuffer;
    class GfxRootSignature;
    class GfxGraphicsPipeline;
    class GfxSampler;
}

namespace gear {
    struct Attachment;
    struct RenderTargetDesc;
    class RenderTarget;
    class CopyEngine;
    class RenderBuiltinResource;
    class RenderView;
    class RenderScene;
    class RenderPassCache;
    class FramebufferCache;
    class GraphicsPipelineCache;
    class Renderer {
    public:
        Renderer();
        ~Renderer();
        void initSurface(void* surface);
        void setSurfaceSizeFunc(std::function<void(int*, int*)> f) { mGetSurfaceSizeFunc = f; }
        void resize(uint32_t width, uint32_t height);
        void render();
        Blast::GfxContext* getContext() { return mContext; }
        Blast::GfxQueue* getQueue() { return mQueue; }
        CopyEngine* getCopyEngine() { return mCopyEngine; }
        RenderScene* getScene() { return mScene; }
        RenderBuiltinResource* getRenderBuiltinResource() { return mRenderBuiltinResource; }
        Blast::ShaderCompiler* getShaderCompiler() { return mShaderCompiler; }
        RenderTarget* createRenderTarget(const RenderTargetDesc& desc);
    private:
        Attachment getColor();
        Attachment getDepthStencil();
        void render(RenderView* view, Blast::GfxCommandBuffer* cmd);
    private:
        friend CopyEngine;
        friend RenderBuiltinResource;
        friend RenderTarget;
        CopyEngine* mCopyEngine;
        RenderBuiltinResource* mRenderBuiltinResource;
        std::function<void(int*, int*)> mGetSurfaceSizeFunc;
        Blast::ShaderCompiler* mShaderCompiler = nullptr;
        Blast::GfxContext* mContext = nullptr;
        Blast::GfxSurface* mSurface = nullptr;
        Blast::GfxSwapchain* mSwapchain = nullptr;
        Blast::GfxRenderPass* mRenderPass = nullptr;
        Blast::GfxFramebuffer** mFramebuffers = nullptr;
        Blast::GfxQueue* mQueue = nullptr;
        Blast::GfxFence** mRenderCompleteFences = nullptr;
        Blast::GfxSemaphore** mImageAcquiredSemaphores = nullptr;
        Blast::GfxSemaphore** mRenderCompleteSemaphores = nullptr;
        Blast::GfxCommandBufferPool* mCmdPool = nullptr;
        Blast::GfxCommandBuffer** mCmds = nullptr;
        Attachment* mColors = nullptr;
        Attachment* mDepthStencils = nullptr;
        RenderTarget* mDefaultRenderTarget = nullptr;
        RenderScene* mScene = nullptr;
        RenderPassCache* mRenderPassCache = nullptr;
        FramebufferCache* mFramebufferCache = nullptr;
        GraphicsPipelineCache* mGraphicsPipelineCache = nullptr;
        uint32_t mFrameIndex = 0;
        uint32_t mImageCount = 0;
    };
}