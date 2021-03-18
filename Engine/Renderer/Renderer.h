#pragma once
#include "Core/GearDefine.h"
#include <functional>

namespace Blast {
    class ShaderCompiler;
    class GfxContext;
    class GfxSurface;
    class GfxSwapchain;
    class GfxRenderPass;
    class GfxQueue;
    class GfxFence;
    class GfxSemaphore;
    class GfxSemaphore;
    class GfxCommandBufferPool;
    class GfxCommandBuffer;
}

namespace gear {
    class Renderer {
    public:
        Renderer();
        ~Renderer();
        void initSurface(void* surface);
        void setSurfaceSizeFunc(std::function<void(int*, int*)> f) { mGetSurfaceSizeFunc = f; }
        void resize(uint32_t width, uint32_t height);
        void render();
    private:
        friend class CopyEngine;
        std::function<void(int*, int*)> mGetSurfaceSizeFunc;
        Blast::ShaderCompiler* mShaderCompiler = nullptr;
        Blast::GfxContext* mContext = nullptr;
        Blast::GfxSurface* mSurface = nullptr;
        Blast::GfxSwapchain* mSwapchain = nullptr;
        Blast::GfxRenderPass** mRenderPasses = nullptr;
        Blast::GfxQueue* mQueue = nullptr;
        Blast::GfxFence** mRenderCompleteFences = nullptr;
        Blast::GfxSemaphore** mImageAcquiredSemaphores = nullptr;
        Blast::GfxSemaphore** mRenderCompleteSemaphores = nullptr;
        Blast::GfxCommandBufferPool* mCmdPool = nullptr;
        Blast::GfxCommandBuffer** mCmds = nullptr;
        uint32_t mFrameIndex = 0;
        uint32_t mImageCount = 0;
    };
}