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

struct Vertex {
    float pos[3];
    float uv[2];
};

namespace gear {
    void Renderer::render(RenderView* view, Blast::GfxCommandBuffer* cmd) {
        view->prepare();

        RenderTarget* presentRT = view->getRenderTarget();
        if (presentRT == nullptr) {
            presentRT = mDefaultRenderTarget;
        }

        Blast::GfxRenderPassDesc renderPassDesc;
        renderPassDesc.numColorAttachments = 1;
        renderPassDesc.colors[0].format = mSurface->getFormat();
        renderPassDesc.colors[0].loadOp = Blast::LOAD_ACTION_CLEAR;
        renderPassDesc.hasDepthStencil = true;
        renderPassDesc.depthStencil.format = Blast::FORMAT_D24_UNORM_S8_UINT;
        renderPassDesc.depthStencil.depthLoadOp = Blast::LOAD_ACTION_CLEAR;
        renderPassDesc.depthStencil.stencilLoadOp = Blast::LOAD_ACTION_CLEAR;
        Blast::GfxRenderPass* presentRP = mRenderPassCache->getRenderPass(renderPassDesc);

        Blast::GfxFramebufferDesc framebufferDesc;
        framebufferDesc.renderPass = presentRP;
        framebufferDesc.width = presentRT->getWidth();
        framebufferDesc.height = presentRT->getHeight();
        framebufferDesc.numColorAttachments = presentRT->getColorTargetCount();
        for (int i = 0; i < framebufferDesc.numColorAttachments; i++) {
            framebufferDesc.colors[i].target = presentRT->getColor(i).texture;
            framebufferDesc.colors[i].layer = presentRT->getColor(i).layer;
            framebufferDesc.colors[i].level = presentRT->getColor(i).level;
        }
        framebufferDesc.hasDepthStencil = presentRT->hasDepthStencil();
        if (framebufferDesc.hasDepthStencil) {
            framebufferDesc.depthStencil.target = presentRT->getDepthStencil().texture;
            framebufferDesc.depthStencil.layer = presentRT->getDepthStencil().layer;
            framebufferDesc.depthStencil.level = presentRT->getDepthStencil().level;
        }
        Blast::GfxFramebuffer* presentFB = mFramebufferCache->getFramebuffer(framebufferDesc);

        Blast::GfxClearValue clearValue;
        clearValue.color[0] = 1.0f;
        clearValue.color[1] = 0.0f;
        clearValue.color[2] = 0.0f;
        clearValue.color[3] = 1.0f;
        clearValue.depth = 1.0f;
        clearValue.stencil = 0;
        cmd->bindRenderTarget(presentRP, presentFB, clearValue);
        cmd->setViewport(0.0, 0.0, presentRT->getWidth(), presentRT->getHeight());
        cmd->setScissor(0, 0, presentRT->getWidth(), presentRT->getHeight());
        for (int i = 0; i < mScene->mPrimitives.size(); ++i) {
            Blast::GfxGraphicsPipelineDesc pipelineDesc;
            Blast::GfxVertexLayout vertexLayout = {};
            vertexLayout.attribCount = 2;
            vertexLayout.attribs[0].semantic = Blast::SEMANTIC_POSITION;
            vertexLayout.attribs[0].format = Blast::FORMAT_R32G32B32_FLOAT;
            vertexLayout.attribs[0].binding = 0;
            vertexLayout.attribs[0].location = 0;
            vertexLayout.attribs[0].offset = 0;

            vertexLayout.attribs[1].semantic = Blast::SEMANTIC_TEXCOORD0;
            vertexLayout.attribs[1].format = Blast::FORMAT_R32G32_FLOAT;
            vertexLayout.attribs[1].binding = 0;
            vertexLayout.attribs[1].location = 1;
            vertexLayout.attribs[1].offset = offsetof(Vertex, uv);

            Blast::GfxBlendState blendState = {};
            blendState.srcFactors[0] = Blast::BLEND_ONE;
            blendState.dstFactors[0] = Blast::BLEND_ZERO;
            blendState.srcAlphaFactors[0] = Blast::BLEND_ONE;
            blendState.dstAlphaFactors[0] = Blast::BLEND_ZERO;
            blendState.masks[0] = 0xf;

            Blast::GfxDepthState depthState = {};
            depthState.depthTest = true;
            depthState.depthWrite = true;

            Blast::GfxRasterizerState rasterizerState = {};
            rasterizerState.cullMode = Blast::CULL_MODE_BACK;
            rasterizerState.frontFace = Blast::FRONT_FACE_CW; // 不再使用gl默认的ccw
            rasterizerState.fillMode = Blast::FILL_MODE_SOLID;
            rasterizerState.primitiveTopo = Blast::PRIMITIVE_TOPO_TRI_LIST;

            pipelineDesc.renderPass = presentRP;
            pipelineDesc.rootSignature = mRenderBuiltinResource->mCustomRootSignature;
            pipelineDesc.vertexShader = mScene->mPrimitives[i]->vertexShader;
            pipelineDesc.pixelShader = mScene->mPrimitives[i]->pixelShader;
            pipelineDesc.vertexLayout = vertexLayout;
            pipelineDesc.blendState = blendState;
            pipelineDesc.depthState = depthState;
            pipelineDesc.rasterizerState = rasterizerState;
            Blast::GfxGraphicsPipeline* pipeline = mGraphicsPipelineCache->getPipeline(pipelineDesc);
            cmd->bindGraphicsPipeline(pipeline);
            cmd->bindRootSignature(mRenderBuiltinResource->mCustomRootSignature);
            cmd->bindVertexBuffer(mScene->mPrimitives[i]->vertexBuffer, 0);
            cmd->bindIndexBuffer(mScene->mPrimitives[i]->indexBuffer, 0, Blast::INDEX_TYPE_UINT32);
            cmd->drawIndexed(6, 1, 0, 0, 0);
        }
        cmd->unbindRenderTarget();
    }
}
