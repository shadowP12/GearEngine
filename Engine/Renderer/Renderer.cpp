#include "Renderer.h"
#include "CopyEngine.h"
#include "RenderBuiltinResource.h"
#include "RenderTarget.h"
#include "RenderScene.h"
#include "RenderCache.h"
#include "Utility/FileSystem.h"
#include <Blast/Gfx/GfxContext.h>
#include <Blast/Gfx/GfxBuffer.h>
#include <Blast/Gfx/GfxTexture.h>
#include <Blast/Gfx/GfxSampler.h>
#include <Blast/Gfx/GfxSwapchain.h>
#include <Blast/Gfx/GfxCommandBuffer.h>
#include <Blast/Gfx/GfxRenderTarget.h>
#include <Blast/Gfx/GfxShader.h>
#include <Blast/Gfx/GfxPipeline.h>
#include <Blast/Gfx/Vulkan/VulkanContext.h>
#include <Blast/Utility/ShaderCompiler.h>
#include <Blast/Utility/VulkanShaderCompiler.h>
#include <assert.h>

namespace gear {
    Renderer::Renderer() {
        mContext = new Blast::VulkanContext();
        mQueue = mContext->getQueue(Blast::QUEUE_TYPE_GRAPHICS);
        Blast::GfxCommandBufferPoolDesc cmdPoolDesc;
        cmdPoolDesc.queue = mQueue;
        cmdPoolDesc.transient = false;
        mCmdPool = mContext->createCommandBufferPool(cmdPoolDesc);
        mCopyEngine = new CopyEngine(this);
        mRenderBuiltinResource = new RenderBuiltinResource(this);
        mDefaultRenderTarget = new RenderTarget(this);
        mScene = new RenderScene(this);
        mRenderPassCache = new RenderPassCache(this);
        mFramebufferCache = new FramebufferCache(this);
        mGraphicsPipelineCache = new GraphicsPipelineCache(this);
        mDescriptorCache = new DescriptorCache(this);

        for (int i = 0; i < UBUFFER_BINDING_COUNT; ++i) {
            mDescriptorKey.uniformBuffers[i] = nullptr;
            mDescriptorKey.uniformBufferOffsets[i] = 0;
            mDescriptorKey.uniformBufferOffsets[i] = 0;
        }

        for (int i = 0; i < SAMPLER_BINDING_COUNT; ++i) {
            mDescriptorKey.textures[i] = nullptr;
            mDescriptorKey.samplers[i] = nullptr;
        }
    }

    Renderer::~Renderer() {
        mQueue->waitIdle();
        SAFE_DELETE(mRenderBuiltinResource);
        SAFE_DELETE(mScene);
        SAFE_DELETE(mRenderPassCache);
        SAFE_DELETE(mFramebufferCache);
        SAFE_DELETE(mGraphicsPipelineCache);
        SAFE_DELETE(mDescriptorCache);
        SAFE_DELETE(mDefaultRenderTarget);
        SAFE_DELETE(mCopyEngine);

        for (int i = 0; i < mImageCount; ++i) {
            SAFE_DELETE(mCmds[i]);
        }
        SAFE_DELETE_ARRAY(mCmds);

        for (int i = 0; i < mImageCount; ++i) {
            SAFE_DELETE(mRenderCompleteFences[i]);
        }
        SAFE_DELETE_ARRAY(mRenderCompleteFences);

        for (int i = 0; i < mImageCount; ++i) {
            SAFE_DELETE(mImageAcquiredSemaphores[i]);
        }
        SAFE_DELETE_ARRAY(mImageAcquiredSemaphores);

        for (int i = 0; i < mImageCount; ++i) {
            SAFE_DELETE(mRenderCompleteSemaphores[i]);
        }
        SAFE_DELETE_ARRAY(mRenderCompleteSemaphores);

        SAFE_DELETE_ARRAY(mColors);
        SAFE_DELETE_ARRAY(mDepthStencils);
        SAFE_DELETE(mCmdPool);
        SAFE_DELETE(mSwapchain);
        SAFE_DELETE(mSurface);
        SAFE_DELETE(mContext);
    }

    void Renderer::terminate() {
        mQueue->waitIdle();
    }

    void Renderer::initSurface(void* surface) {
        Blast::GfxSurfaceDesc surfaceDesc;
        surfaceDesc.originSurface = surface;
        mSurface = mContext->createSurface(surfaceDesc);
    }

    RenderTarget* Renderer::createRenderTarget(const RenderTargetDesc& desc) {
        return new RenderTarget(this, desc);
    }

    Attachment Renderer::getColor() {
        return mColors[mFrameIndex];
    }

    Attachment Renderer::getDepthStencil() {
        return mDepthStencils[mFrameIndex];
    }

    void Renderer::resize(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) {
            return;
        }
        mQueue->waitIdle();
        Blast::GfxSwapchain* oldSwapchain = mSwapchain;
        Blast::GfxSwapchainDesc swapchainDesc;
        swapchainDesc.width = width;
        swapchainDesc.height = height;
        swapchainDesc.surface = mSurface;
        swapchainDesc.oldSwapchain = oldSwapchain;
        mSwapchain = mContext->createSwapchain(swapchainDesc);

        SAFE_DELETE(oldSwapchain);

        for (int i = 0; i < mImageCount; ++i) {
            SAFE_DELETE(mCmds[i]);
        }
        SAFE_DELETE_ARRAY(mCmds);

        for (int i = 0; i < mImageCount; ++i) {
            SAFE_DELETE(mRenderCompleteFences[i]);
        }
        SAFE_DELETE_ARRAY(mRenderCompleteFences);

        for (int i = 0; i < mImageCount; ++i) {
            SAFE_DELETE(mImageAcquiredSemaphores[i]);
        }
        SAFE_DELETE_ARRAY(mImageAcquiredSemaphores);

        for (int i = 0; i < mImageCount; ++i) {
            SAFE_DELETE(mRenderCompleteSemaphores[i]);
        }
        SAFE_DELETE_ARRAY(mRenderCompleteSemaphores);

        SAFE_DELETE_ARRAY(mColors);

        SAFE_DELETE_ARRAY(mDepthStencils);

        mImageCount = mSwapchain->getImageCount();

        mRenderCompleteFences = new Blast::GfxFence*[mImageCount];
        mImageAcquiredSemaphores = new Blast::GfxSemaphore*[mImageCount];
        mRenderCompleteSemaphores = new Blast::GfxSemaphore*[mImageCount];
        mCmds = new Blast::GfxCommandBuffer*[mImageCount];
        mColors = new Attachment[mImageCount];
        mDepthStencils = new Attachment[mImageCount];
        for (int i = 0; i < mImageCount; ++i) {
            // sync
            mRenderCompleteFences[i] = mContext->createFence();
            mImageAcquiredSemaphores[i] = mContext->createSemaphore();
            mRenderCompleteSemaphores[i] = mContext->createSemaphore();

            // attachments
            mColors[i].level = 0;
            mColors[i].layer = 0;
            mColors[i].texture = mSwapchain->getColorRenderTarget(i);
            mColors[i].format = mSurface->getFormat();

            mDepthStencils[i].level = 0;
            mDepthStencils[i].layer = 0;
            mDepthStencils[i].texture = mSwapchain->getDepthRenderTarget(i);
            mDepthStencils[i].format = Blast::FORMAT_D24_UNORM_S8_UINT;

            // cmd
            mCmds[i] = mCmdPool->allocBuf(false);

            // set present format
            Blast::GfxTexture* colorRT = mSwapchain->getColorRenderTarget(i);
            Blast::GfxTexture* depthRT = mSwapchain->getDepthRenderTarget(i);
            mCmds[i]->begin();
            {
                // 设置交换链RT为显示状态
                Blast::GfxTextureBarrier barriers[2];
                barriers[0].texture = colorRT;
                barriers[0].newState = Blast::RESOURCE_STATE_PRESENT ;
                barriers[1].texture = depthRT;
                barriers[1].newState = Blast::RESOURCE_STATE_DEPTH_WRITE ;
                mCmds[i]->setBarrier(0, nullptr, 2, barriers);
            }
            mCmds[i]->end();
        }
        Blast::GfxSubmitInfo submitInfo;
        submitInfo.cmdBufCount = mImageCount;
        submitInfo.cmdBufs = mCmds;
        submitInfo.signalFence = nullptr;
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.waitSemaphores = nullptr;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.signalSemaphores = nullptr;
        mQueue->submit(submitInfo);
        mQueue->waitIdle();
    }

    void Renderer::beginFrame(uint32_t width, uint32_t height) {
        if (mFrameWidth != width || mFrameHeight != height) {
            mFrameWidth = width;
            mFrameHeight = height;
            resize(mFrameWidth, mFrameHeight);
        }

        uint32_t swapchainImageIndex;
        mContext->acquireNextImage(mSwapchain, mImageAcquiredSemaphores[mFrameIndex], nullptr, &swapchainImageIndex);
        if (swapchainImageIndex == -1) {
            return;
        }

        mRenderCompleteFences[mFrameIndex]->waitForComplete();

        Blast::GfxTexture* colorRT = mSwapchain->getColorRenderTarget(mFrameIndex);
        Blast::GfxTexture* depthRT = mSwapchain->getDepthRenderTarget(mFrameIndex);

        mCmds[mFrameIndex]->begin();
        {
            // 设置交换链RT为可写状态
            Blast::GfxTextureBarrier barriers[2];
            barriers[0].texture = colorRT;
            barriers[0].newState = Blast::RESOURCE_STATE_RENDER_TARGET;
            barriers[1].texture = depthRT;
            barriers[1].newState = Blast::RESOURCE_STATE_DEPTH_WRITE;
            mCmds[mFrameIndex]->setBarrier(0, nullptr, 2, barriers);
        }

        // 初始化渲染器场景
        mScene->prepare();

        // 绘制每一个RenderView
        for (int i = 0; i < mScene->mViewCount; ++i) {
            render(&mScene->mViews[i], mCmds[mFrameIndex]);
        }

        {
            // 设置交换链RT为显示状态
            Blast::GfxTextureBarrier barriers[2];
            barriers[0].texture = colorRT;
            barriers[0].newState = Blast::RESOURCE_STATE_PRESENT ;
            barriers[1].texture = depthRT;
            barriers[1].newState = Blast::RESOURCE_STATE_DEPTH_WRITE ;
            mCmds[mFrameIndex]->setBarrier(0, nullptr, 2, barriers);
        }
        mCmds[mFrameIndex]->end();

        Blast::GfxSubmitInfo submitInfo;
        submitInfo.cmdBufCount = 1;
        submitInfo.cmdBufs = &mCmds[mFrameIndex];
        submitInfo.signalFence = mRenderCompleteFences[mFrameIndex];
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.waitSemaphores = &mImageAcquiredSemaphores[mFrameIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.signalSemaphores = &mRenderCompleteSemaphores[mFrameIndex];
        mQueue->submit(submitInfo);

        Blast::GfxPresentInfo presentInfo;
        presentInfo.swapchain = mSwapchain;
        presentInfo.index = swapchainImageIndex;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.waitSemaphores = &mRenderCompleteSemaphores[mFrameIndex];
        mQueue->present(presentInfo);
        mFrameIndex = (mFrameIndex + 1) % mImageCount;
    }

    void Renderer::endFrame() {
        mCopyEngine->update();
    }

    void Renderer::prepare() {

    }

}