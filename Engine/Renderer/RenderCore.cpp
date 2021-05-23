#include "Renderer.h"
#include "RenderTarget.h"
#include "RenderCache.h"
#include "RenderScene.h"
#include "RenderBuiltinResource.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Material.h"
#include <Blast/Gfx/GfxRenderTarget.h>
#include <Blast/Gfx/GfxCommandBuffer.h>
namespace gear {
    void Renderer::bindRenderTarget(RenderTarget* rt, RenderTargetParams* params) {
        Blast::GfxCommandBuffer* cmd = mCmds[mFrameIndex];
        Blast::GfxRenderPassDesc renderPassDesc;
        renderPassDesc.numColorAttachments = rt->getColorTargetCount();
        for (int i = 0; i < rt->getColorTargetCount(); i++) {
            renderPassDesc.colors[i].format = rt->getColor(i).format;
            renderPassDesc.colors[i].loadOp = Blast::LOAD_ACTION_CLEAR;
        }
        if (rt->hasDepthStencil()) {
            renderPassDesc.hasDepthStencil = true;
            renderPassDesc.depthStencil.format = rt->getDepthStencil().format;
            renderPassDesc.depthStencil.depthLoadOp = Blast::LOAD_ACTION_CLEAR;
            renderPassDesc.depthStencil.stencilLoadOp = Blast::LOAD_ACTION_CLEAR;
        }
        Blast::GfxRenderPass* rp = mRenderPassCache->getRenderPass(renderPassDesc);

        Blast::GfxFramebufferDesc framebufferDesc;
        framebufferDesc.renderPass = rp;
        framebufferDesc.width = rt->getWidth();
        framebufferDesc.height = rt->getHeight();
        framebufferDesc.numColorAttachments = rt->getColorTargetCount();
        for (int i = 0; i < framebufferDesc.numColorAttachments; i++) {
            framebufferDesc.colors[i].target = rt->getColor(i).texture;
            framebufferDesc.colors[i].layer = rt->getColor(i).layer;
            framebufferDesc.colors[i].level = rt->getColor(i).level;
        }
        framebufferDesc.hasDepthStencil = rt->hasDepthStencil();
        if (framebufferDesc.hasDepthStencil) {
            framebufferDesc.depthStencil.target = rt->getDepthStencil().texture;
            framebufferDesc.depthStencil.layer = rt->getDepthStencil().layer;
            framebufferDesc.depthStencil.level = rt->getDepthStencil().level;
        }
        Blast::GfxFramebuffer* fb = mFramebufferCache->getFramebuffer(framebufferDesc);

        // TODO: 设置RenderTarget相关参数
        Blast::GfxClearValue clearValue;
        clearValue.color[0] = 1.0f;
        clearValue.color[1] = 0.0f;
        clearValue.color[2] = 0.0f;
        clearValue.color[3] = 1.0f;
        clearValue.depth = 1.0f;
        clearValue.stencil = 0;
        cmd->bindRenderTarget(rp, fb, clearValue);
        cmd->setViewport(0.0, 0.0, rt->getWidth(), rt->getHeight());
        cmd->setScissor(0, 0, rt->getWidth(), rt->getHeight());

        // 设置当前已经绑定好的Render Pass
        mBindRenderPass = rp;
    }

    void Renderer::unbindRenderTarget() {
        Blast::GfxCommandBuffer* cmd = mCmds[mFrameIndex];
        cmd->unbindRenderTarget();
    }

    void Renderer::executeDrawCall(DrawCall* dc) {
        Blast::GfxCommandBuffer* cmd = mCmds[mFrameIndex];
        Renderable* rb = &mScene->mRenderables[dc->index];

        Blast::GfxGraphicsPipelineDesc pipelineDesc;

        pipelineDesc.renderPass = mBindRenderPass;
        pipelineDesc.rootSignature = mRenderBuiltinResource->mCustomRootSignature;
        pipelineDesc.vertexShader = rb->materialInstance->getMaterial()->getVertShader(0);
        pipelineDesc.pixelShader = rb->materialInstance->getMaterial()->getFragShader(0);
        pipelineDesc.vertexLayout = rb->vertexBuffer->getVertexLayout();
        pipelineDesc.blendState = dc->blendState;
        pipelineDesc.depthState = dc->depthState;
        pipelineDesc.rasterizerState = dc->rasterizerState;
        Blast::GfxGraphicsPipeline* pipeline = mGraphicsPipelineCache->getPipeline(pipelineDesc);
        cmd->bindGraphicsPipeline(pipeline);
        // cmd->bindRootSignature(mRenderBuiltinResource->mCustomRootSignature);
        cmd->bindVertexBuffer(rb->vertexBuffer->getBuffer(), 0);
        cmd->bindIndexBuffer(rb->indexBuffer->getBuffer(), 0, rb->indexBuffer->getIndexType());
        cmd->drawIndexed(rb->count, 1, 0, 0, 0);
    }
}