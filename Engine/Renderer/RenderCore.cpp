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
            if (params->clearColor) {
                renderPassDesc.colors[i].loadOp = Blast::LOAD_ACTION_CLEAR;
            }
            else {
                renderPassDesc.colors[i].loadOp = Blast::LOAD_ACTION_LOAD;
            }
        }
        if (rt->hasDepthStencil()) {
            renderPassDesc.hasDepthStencil = true;
            renderPassDesc.depthStencil.format = rt->getDepthStencil().format;
            if (params->clearDepth) {
                renderPassDesc.depthStencil.depthLoadOp = Blast::LOAD_ACTION_CLEAR;
            } else {
                renderPassDesc.depthStencil.depthLoadOp = Blast::LOAD_ACTION_LOAD;
            }

            if (params->clearStencil) {
                renderPassDesc.depthStencil.stencilLoadOp = Blast::LOAD_ACTION_CLEAR;
            } else {
                renderPassDesc.depthStencil.stencilLoadOp = Blast::LOAD_ACTION_LOAD;
            }
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

        Blast::GfxGraphicsPipelineDesc pipelineDesc;

        // 绑定materialUB
        if (dc->materialInstance->mUniformBuffer) {
            mDescriptorKey.uniformBuffers[0] = dc->materialInstance->mUniformBuffer->getBuffer();
            mDescriptorKey.uniformBufferSizes[0] = dc->materialInstance->mUniformBufferSize;
            mDescriptorKey.uniformBufferOffsets[0] = 0;
        }

        // 绑定renderableUB
        mDescriptorKey.uniformBuffers[2] = dc->renderableUB->getBuffer();
        mDescriptorKey.uniformBufferSizes[2] = sizeof(ObjectUniforms);
        mDescriptorKey.uniformBufferOffsets[2] = 0;

        // 绑定materialSamplers
        for(auto iter = dc->materialInstance->mSamplerGroup.begin(); iter != dc->materialInstance->mSamplerGroup.end(); iter++) {
            mDescriptorKey.textures[iter->first] = iter->second.texture->getTexture();
            mDescriptorKey.samplers[iter->first] = mSamplerCache->getSampler(iter->second.params);
        }

        DescriptorBundle descriptorBundle = mDescriptorCache->getDescriptor(mDescriptorKey);

        pipelineDesc.renderPass = mBindRenderPass;
        pipelineDesc.rootSignature = mRenderBuiltinResource->mCustomRootSignature;
        pipelineDesc.vertexShader = dc->materialInstance->getMaterial()->getVertShader(0);
        pipelineDesc.pixelShader = dc->materialInstance->getMaterial()->getFragShader(0);
        pipelineDesc.vertexLayout = dc->vertexBuffer->getVertexLayout();
        pipelineDesc.blendState = dc->blendState;
        pipelineDesc.depthState = dc->depthState;
        pipelineDesc.rasterizerState = dc->rasterizerState;
        Blast::GfxGraphicsPipeline* pipeline = mGraphicsPipelineCache->getPipeline(pipelineDesc);
        cmd->bindGraphicsPipeline(pipeline);
        cmd->bindRootSignature(mRenderBuiltinResource->mCustomRootSignature);
        cmd->bindDescriptorSets(2, descriptorBundle.handles);
        cmd->bindVertexBuffer(dc->vertexBuffer->getBuffer(), 0);
        cmd->bindIndexBuffer(dc->indexBuffer->getBuffer(), 0, dc->indexBuffer->getIndexType());
        cmd->drawIndexed(dc->count, 1, dc->offset, 0, 0);
    }
}