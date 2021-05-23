#include "RenderCache.h"
#include "Renderer.h"
#include <Blast/Gfx/GfxContext.h>

namespace gear {
    RenderPassCache::RenderPassCache(Renderer* renderer) {
        mRenderer = renderer;
    }

    RenderPassCache::~RenderPassCache() {
        for (auto iter = mRenderPasses.begin(); iter != mRenderPasses.end(); ++iter) {
            SAFE_DELETE(iter->second);
        }
        mRenderPasses.clear();
    }

    bool RenderPassCache::RenderPassEq::operator()(const Blast::GfxRenderPassDesc& desc1, const Blast::GfxRenderPassDesc& desc2) const {
        if (desc1.numColorAttachments != desc2.numColorAttachments) return false;
        if (desc1.hasDepthStencil != desc2.hasDepthStencil) return false;
        if (desc1.depthStencil.sampleCount != desc2.depthStencil.sampleCount) return false;
        if (desc1.depthStencil.stencilLoadOp != desc2.depthStencil.stencilLoadOp) return false;
        if (desc1.depthStencil.depthLoadOp != desc2.depthStencil.depthLoadOp) return false;
        if (desc1.depthStencil.format != desc2.depthStencil.format) return false;
        for (int i = 0; i < MAX_RENDER_TARGET_ATTACHMENTS; i++) {
            if (desc1.colors[i].sampleCount != desc2.colors[i].sampleCount) return false;
            if (desc1.colors[i].format != desc2.colors[i].format) return false;
            if (desc1.colors[i].loadOp != desc2.colors[i].loadOp) return false;
        }
        return true;
    }

    Blast::GfxRenderPass* RenderPassCache::getRenderPass(const Blast::GfxRenderPassDesc& desc) {
        auto iter = mRenderPasses.find(desc);
        if (iter != mRenderPasses.end()) {
            return iter->second;
        }

        Blast::GfxRenderPass* renderPass = mRenderer->getContext()->createRenderPass(desc);
        mRenderPasses[desc] = renderPass;
        return renderPass;
    }

    FramebufferCache::FramebufferCache(Renderer* renderer) {
        mRenderer = renderer;
    }

    FramebufferCache::~FramebufferCache() {
        for (auto iter = mFrambuffers.begin(); iter != mFrambuffers.end(); ++iter) {
            SAFE_DELETE(iter->second);
        }
        mFrambuffers.clear();
    }

    bool FramebufferCache::FramebufferEq::operator()(const Blast::GfxFramebufferDesc& desc1, const Blast::GfxFramebufferDesc& desc2) const {
        if (desc1.width != desc2.width) return false;
        if (desc1.height != desc2.height) return false;
        if (desc1.renderPass != desc2.renderPass) return false;
        if (desc1.numColorAttachments != desc2.numColorAttachments) return false;
        if (desc1.hasDepthStencil != desc2.hasDepthStencil) return false;
        if (desc1.depthStencil.target != desc2.depthStencil.target) return false;
        if (desc1.depthStencil.level != desc2.depthStencil.level) return false;
        if (desc1.depthStencil.layer != desc2.depthStencil.layer) return false;
        for (int i = 0; i < MAX_RENDER_TARGET_ATTACHMENTS; i++) {
            if (desc1.colors[i].target != desc2.colors[i].target) return false;
            if (desc1.colors[i].level != desc2.colors[i].level) return false;
            if (desc1.colors[i].layer != desc2.colors[i].layer) return false;
        }
        return true;
    }

    Blast::GfxFramebuffer* FramebufferCache::getFramebuffer(const Blast::GfxFramebufferDesc& desc) {
        auto iter = mFrambuffers.find(desc);
        if (iter != mFrambuffers.end()) {
            return iter->second;
        }

        Blast::GfxFramebuffer* framebuffer = mRenderer->getContext()->createFramebuffer(desc);
        mFrambuffers[desc] = framebuffer;
        return framebuffer;
    }

    GraphicsPipelineCache::GraphicsPipelineCache(Renderer* renderer) {
        mRenderer = renderer;
    }

    GraphicsPipelineCache::~GraphicsPipelineCache() {
        for (auto iter = mPipelines.begin(); iter != mPipelines.end(); ++iter) {
            SAFE_DELETE(iter->second);
        }
        mPipelines.clear();
    }

    bool GraphicsPipelineCache::PipelineEq::operator()(const Blast::GfxGraphicsPipelineDesc& desc1, const Blast::GfxGraphicsPipelineDesc& desc2) const {
        if (desc1.renderPass != desc2.renderPass) return false;
        if (desc1.vertexShader != desc2.vertexShader) return false;
        if (desc1.hullShader != desc2.hullShader) return false;
        if (desc1.domainShader != desc2.domainShader) return false;
        if (desc1.geometryShader != desc2.geometryShader) return false;
        if (desc1.pixelShader != desc2.pixelShader) return false;
        if (desc1.rootSignature != desc2.rootSignature) return false;

        // Vertex Layout
        if (desc1.vertexLayout.attribCount != desc2.vertexLayout.attribCount) return false;
        for (int i = 0; i < MAX_VERTEX_ATTRIBS; i++) {
            if (desc1.vertexLayout.attribs[i].format != desc2.vertexLayout.attribs[i].format) return false;
            if (desc1.vertexLayout.attribs[i].semantic != desc2.vertexLayout.attribs[i].semantic) return false;
            if (desc1.vertexLayout.attribs[i].rate != desc2.vertexLayout.attribs[i].rate) return false;
            if (desc1.vertexLayout.attribs[i].location != desc2.vertexLayout.attribs[i].location) return false;
            if (desc1.vertexLayout.attribs[i].binding != desc2.vertexLayout.attribs[i].binding) return false;
            if (desc1.vertexLayout.attribs[i].offset != desc2.vertexLayout.attribs[i].offset) return false;
        }

        // Blend State
        if (desc1.blendState.independentBlend != desc2.blendState.independentBlend) return false;
        if (desc1.blendState.targetMask != desc2.blendState.targetMask) return false;
        for (int i = 0; i < MAX_RENDER_TARGET_ATTACHMENTS; i++) {
            if (desc1.blendState.blendAlphaOps[i] != desc2.blendState.blendAlphaOps[i]) return false;
            if (desc1.blendState.blendOps[i] != desc2.blendState.blendOps[i]) return false;
            if (desc1.blendState.srcAlphaFactors[i] != desc2.blendState.srcAlphaFactors[i]) return false;
            if (desc1.blendState.dstAlphaFactors[i] != desc2.blendState.dstAlphaFactors[i]) return false;
            if (desc1.blendState.srcFactors[i] != desc2.blendState.srcFactors[i]) return false;
            if (desc1.blendState.dstFactors[i] != desc2.blendState.dstFactors[i]) return false;
            if (desc1.blendState.masks[i] != desc2.blendState.masks[i]) return false;
        }

        // Depth State
        if (desc1.depthState.depthTest != desc2.depthState.depthTest) return false;
        if (desc1.depthState.depthWrite != desc2.depthState.depthWrite) return false;
        if (desc1.depthState.depthFunc != desc2.depthState.depthFunc) return false;
        if (desc1.depthState.stencilTest != desc2.depthState.stencilTest) return false;
        if (desc1.depthState.stencilReadMask != desc2.depthState.stencilReadMask) return false;
        if (desc1.depthState.stencilWriteMask != desc2.depthState.stencilWriteMask) return false;
        if (desc1.depthState.stencilFrontFunc != desc2.depthState.stencilFrontFunc) return false;
        if (desc1.depthState.stencilFrontFail != desc2.depthState.stencilFrontFail) return false;
        if (desc1.depthState.depthFrontFail != desc2.depthState.depthFrontFail) return false;
        if (desc1.depthState.stencilFrontPass != desc2.depthState.stencilFrontPass) return false;
        if (desc1.depthState.stencilBackFunc != desc2.depthState.stencilBackFunc) return false;
        if (desc1.depthState.stencilBackFail != desc2.depthState.stencilBackFail) return false;
        if (desc1.depthState.depthBackFail != desc2.depthState.depthBackFail) return false;
        if (desc1.depthState.stencilBackPass != desc2.depthState.stencilBackPass) return false;

        // Rasterizer State
        if (desc1.rasterizerState.depthBias != desc2.rasterizerState.depthBias) return false;
        if (desc1.rasterizerState.slopeScaledDepthBias != desc2.rasterizerState.slopeScaledDepthBias) return false;
        if (desc1.rasterizerState.multiSample != desc2.rasterizerState.multiSample) return false;
        if (desc1.rasterizerState.depthClampEnable != desc2.rasterizerState.depthClampEnable) return false;
        if (desc1.rasterizerState.primitiveTopo != desc2.rasterizerState.primitiveTopo) return false;
        if (desc1.rasterizerState.fillMode != desc2.rasterizerState.fillMode) return false;
        if (desc1.rasterizerState.frontFace != desc2.rasterizerState.frontFace) return false;
        if (desc1.rasterizerState.cullMode != desc2.rasterizerState.cullMode) return false;

        return true;
    }

    Blast::GfxGraphicsPipeline* GraphicsPipelineCache::getPipeline(const Blast::GfxGraphicsPipelineDesc& desc) {
        auto iter = mPipelines.find(desc);
        if (iter != mPipelines.end()) {
            return iter->second;
        }

        Blast::GfxGraphicsPipeline* pipeline = mRenderer->getContext()->createGraphicsPipeline(desc);
        mPipelines[desc] = pipeline;
        return pipeline;
    }
}