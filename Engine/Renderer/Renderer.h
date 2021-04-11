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
    class CopyEngine;
    class RenderBuiltinResource;
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
        RenderBuiltinResource* getRenderBuiltinResource() { return mRenderBuiltinResource; }
        Attachment getColor();
        Attachment getDepthStencil();
    private:
        friend CopyEngine;
        friend RenderBuiltinResource;
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
        uint32_t mFrameIndex = 0;
        uint32_t mImageCount = 0;
    };
}