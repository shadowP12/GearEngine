#include "Renderer.h"
#include "RenderTarget.h"
#include "RenderCache.h"
#include "RenderScene.h"
#include "RenderBuiltinResource.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Material.h"
#include "Resource/Texture.h"
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
        clearValue.color[0] = 0.0f;
        clearValue.color[1] = 0.0f;
        clearValue.color[2] = 0.0f;
        clearValue.color[3] = 0.0f;
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

        dc->blendState.srcFactors[0] = Blast::BLEND_ONE;
        dc->blendState.dstFactors[0] = Blast::BLEND_ZERO;
        dc->blendState.srcAlphaFactors[0] = Blast::BLEND_ONE;
        dc->blendState.dstAlphaFactors[0] = Blast::BLEND_ZERO;
        dc->blendState.masks[0] = 0xf;

        // 绑定materialUB
        if (rb->materialInstance->mUniformBuffer) {
            mDescriptorKey.uniformBuffers[0] = rb->materialInstance->mUniformBuffer->getBuffer();
            mDescriptorKey.uniformBufferSizes[0] = rb->materialInstance->mUniformBufferSize;
            mDescriptorKey.uniformBufferOffsets[0] = 0;
        }

        // 绑定renderableUB
        mDescriptorKey.uniformBuffers[2] = rb->renderableUB->getBuffer();
        mDescriptorKey.uniformBufferSizes[2] = sizeof(ObjectUniforms);
        mDescriptorKey.uniformBufferOffsets[2] = 0;

        // 绑定materialSamplers
        for(auto iter = rb->materialInstance->mSamplerGroup.begin(); iter != rb->materialInstance->mSamplerGroup.end(); iter++) {
            mDescriptorKey.textures[iter->first] = iter->second.texture->getTexture();
            mDescriptorKey.samplers[iter->first] = mSamplerCache->getSampler(iter->second.params);
        }

        DescriptorBundle descriptorBundle = mDescriptorCache->getDescriptor(mDescriptorKey);

//        dc->depthState.depthTest = true;
//        dc->depthState.depthWrite = true;

//        dc->rasterizerState.cullMode = Blast::CULL_MODE_BACK;
//        dc->rasterizerState.frontFace = Blast::FRONT_FACE_CW; // 不再使用gl默认的ccw
//        dc->rasterizerState.fillMode = Blast::FILL_MODE_SOLID;
//        dc->rasterizerState.primitiveTopo = Blast::PRIMITIVE_TOPO_TRI_LIST;

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
        cmd->bindRootSignature(mRenderBuiltinResource->mCustomRootSignature);
        cmd->bindDescriptorSets(2, descriptorBundle.handles);
        cmd->bindVertexBuffer(rb->vertexBuffer->getBuffer(), 0);
        cmd->bindIndexBuffer(rb->indexBuffer->getBuffer(), 0, rb->indexBuffer->getIndexType());
        cmd->drawIndexed(rb->count, 1, 0, 0, 0);
    }
}