#include "Renderer.h"
#include <Gfx/GfxContext.h>
#include <Gfx/GfxBuffer.h>
#include <Gfx/GfxTexture.h>
#include <Gfx/GfxSampler.h>
#include <Gfx/GfxSwapchain.h>
#include <Gfx/GfxCommandBuffer.h>
#include <Gfx/GfxRenderPass.h>
#include <Gfx/GfxShader.h>
#include <Gfx/GfxPipeline.h>
#include <Gfx/Vulkan/VulkanContext.h>
#include <Utility/ShaderCompiler.h>
#include <Utility/VulkanShaderCompiler.h>

namespace gear {
    Renderer::Renderer() {
        mShaderCompiler = new Blast::VulkanShaderCompiler();
        mContext = new Blast::VulkanContext();
        mQueue = mContext->getQueue(Blast::QUEUE_TYPE_GRAPHICS);
        Blast::GfxCommandBufferPoolDesc cmdPoolDesc;
        cmdPoolDesc.queue = mQueue;
        cmdPoolDesc.transient = false;
        mCmdPool = mContext->createCommandBufferPool(cmdPoolDesc);
    }

    Renderer::~Renderer() {
        mQueue->waitIdle();
        SAFE_DELETE_ARRAY(mCmds);
        SAFE_DELETE_ARRAY(mRenderCompleteFences);
        SAFE_DELETE_ARRAY(mImageAcquiredSemaphores);
        SAFE_DELETE_ARRAY(mRenderCompleteSemaphores);
        SAFE_DELETE_ARRAY(mRenderPasses);
        SAFE_DELETE(mCmdPool);
        SAFE_DELETE(mQueue);
        SAFE_DELETE(mSurface);
        SAFE_DELETE(mSwapchain);
        SAFE_DELETE(mContext);
        SAFE_DELETE(mShaderCompiler);
    }

    void Renderer::initSurface(void* surface) {
        Blast::GfxSurfaceDesc surfaceDesc;
        surfaceDesc.originSurface = surface;
        mSurface = mContext->createSurface(surfaceDesc);
    }

    void Renderer::resize(uint32_t width, uint32_t height) {
        mQueue->waitIdle();
        if (width == 0 || height == 0) {
            return;
        }
        Blast::GfxSwapchain* oldSwapchain = mSwapchain;
        Blast::GfxSwapchainDesc swapchainDesc;
        swapchainDesc.width = width;
        swapchainDesc.height = height;
        swapchainDesc.surface = mSurface;
        swapchainDesc.oldSwapchain = oldSwapchain;
        mSwapchain = mContext->createSwapchain(swapchainDesc);

        SAFE_DELETE(oldSwapchain);
        SAFE_DELETE_ARRAY(mCmds);
        SAFE_DELETE_ARRAY(mRenderCompleteFences);
        SAFE_DELETE_ARRAY(mImageAcquiredSemaphores);
        SAFE_DELETE_ARRAY(mRenderCompleteSemaphores);
        SAFE_DELETE_ARRAY(mRenderPasses);

        mImageCount = mSwapchain->getImageCount();

        mRenderCompleteFences = new Blast::GfxFence*[mImageCount];
        mImageAcquiredSemaphores = new Blast::GfxSemaphore*[mImageCount];
        mRenderCompleteSemaphores = new Blast::GfxSemaphore*[mImageCount];
        mCmds = new Blast::GfxCommandBuffer*[mImageCount];
        mRenderPasses = new Blast::GfxRenderPass*[mImageCount];
        for (int i = 0; i < mImageCount; ++i) {
            // sync
            mRenderCompleteFences[i] = mContext->createFence();
            mImageAcquiredSemaphores[i] = mContext->createSemaphore();
            mRenderCompleteSemaphores[i] = mContext->createSemaphore();

            // renderPassws
            Blast::GfxRenderPassDesc renderPassDesc;
            renderPassDesc.numColorAttachments = 1;
            renderPassDesc.colors[0].target = mSwapchain->getColorRenderTarget(i);
            renderPassDesc.colors[0].loadOp = Blast::LOAD_ACTION_CLEAR;
            renderPassDesc.hasDepthStencil = true;
            renderPassDesc.depthStencil.target = mSwapchain->getDepthRenderTarget(i);
            renderPassDesc.depthStencil.depthLoadOp = Blast::LOAD_ACTION_CLEAR;
            renderPassDesc.depthStencil.stencilLoadOp = Blast::LOAD_ACTION_CLEAR;
            renderPassDesc.width = width;
            renderPassDesc.height = height;
            mRenderPasses[i] = mContext->createRenderPass(renderPassDesc);

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

    void Renderer::render() {
        uint32_t swapchainImageIndex;
        mContext->acquireNextImage(mSwapchain, mImageAcquiredSemaphores[mFrameIndex], nullptr, &swapchainImageIndex);
        if (swapchainImageIndex == -1) {
            // note: resize
            int width, height;
            mGetSurfaceSizeFunc(&width, &height);
            resize(width, height);
            return;
        }

        mRenderCompleteFences[mFrameIndex]->waitForComplete();
        mRenderCompleteFences[mFrameIndex]->reset();

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
        // 渲染UI
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
        int ret = mQueue->present(presentInfo);
        mFrameIndex = (mFrameIndex + 1) % mImageCount;
    }
}