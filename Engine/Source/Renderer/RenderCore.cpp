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

    }
}