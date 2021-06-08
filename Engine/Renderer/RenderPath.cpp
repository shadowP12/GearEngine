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
        for (int i = 0; i < view->renderableCount; ++i) {
            Renderable* rb = &view->renderables[i];
            for (int j = 0; j < rb->primitives.size(); ++j) {
                RenderPrimitive* rp = &rb->primitives[j];
                mDrawCalls[mDrawCallHead + colorDrawCallCount].variant = 0;
                mDrawCalls[mDrawCallHead + colorDrawCallCount].vertexBuffer = rb->vertexBuffer;
                mDrawCalls[mDrawCallHead + colorDrawCallCount].indexBuffer = rb->indexBuffer;
                mDrawCalls[mDrawCallHead + colorDrawCallCount].renderableUB = rb->renderableUB;
                mDrawCalls[mDrawCallHead + colorDrawCallCount].boneUB = rb->boneUB;
                mDrawCalls[mDrawCallHead + colorDrawCallCount].count = rp->count;
                mDrawCalls[mDrawCallHead + colorDrawCallCount].offset = rp->offset;
                mDrawCalls[mDrawCallHead + colorDrawCallCount].type = rp->type;
                mDrawCalls[mDrawCallHead + colorDrawCallCount].materialInstance = rp->materialInstance;
                mDrawCalls[mDrawCallHead + colorDrawCallCount].blendState = rp->materialInstance->getMaterial()->mBlendState;
                mDrawCalls[mDrawCallHead + colorDrawCallCount].depthState = rp->materialInstance->getMaterial()->mDepthState;
                mDrawCalls[mDrawCallHead + colorDrawCallCount].rasterizerState = rp->materialInstance->getMaterial()->mRasterizerState;
                colorDrawCallCount++;
            }
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
