#include "Renderer.h"
#include "CopyEngine.h"
#include "RenderBuiltinResource.h"
#include "RenderTarget.h"
#include "RenderScene.h"
#include "RenderCache.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Material.h"
#include "Resource/Texture.h"
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
#include <algorithm>

struct Vertex {
    float pos[3];
    float uv[2];
};

namespace gear {
    void Renderer::render(RenderView* view, Blast::GfxCommandBuffer* cmd) {
        // 在渲染开始前清空所有渲染命令缓存
        mDrawCallHead = 0;
        mDescriptorKey.uniformBuffers[1] = view->cameraUB->getBuffer();
        mDescriptorKey.uniformBufferSizes[1] = sizeof(FrameUniforms);
        mDescriptorKey.uniformBufferOffsets[1] = 0;

        // 填充
        uint32_t colorDrawCallCount = 0;
        for (int i = 0; i < mScene->mRenderableCount; ++i) {
            Renderable* rb = &mScene->mRenderables[i];
            mDrawCalls[mDrawCallHead + colorDrawCallCount].blendState = rb->materialInstance->getMaterial()->mBlendState;
            mDrawCalls[mDrawCallHead + colorDrawCallCount].depthState = rb->materialInstance->getMaterial()->mDepthState;
            mDrawCalls[mDrawCallHead + colorDrawCallCount].rasterizerState = rb->materialInstance->getMaterial()->mRasterizerState;
            mDrawCalls[mDrawCallHead + colorDrawCallCount].index = i;
            colorDrawCallCount++;
        }

        // 排序
        std::sort(&mDrawCalls[mDrawCallHead], &mDrawCalls[mDrawCallHead] + colorDrawCallCount);

        // 执行
        RenderTargetParams params;
        bindRenderTarget(view->renderTarget, &params);
        for (int i = mDrawCallHead; i < colorDrawCallCount; ++i) {
            executeDrawCall(&mDrawCalls[i]);
        }
        unbindRenderTarget();
    }
}
