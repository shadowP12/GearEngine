#pragma once
#include "Core/GearDefine.h"
#include <Blast/Gfx/GfxDefine.h>
#include <Blast/Gfx/GfxRenderTarget.h>
#include <Blast/Gfx/GfxPipeline.h>
#include <map>

namespace gear {
    class Renderer;
    class RenderPassCache {
    public:
        RenderPassCache(Renderer* renderer);
        ~RenderPassCache();
        Blast::GfxRenderPass* getRenderPass(const Blast::GfxRenderPassDesc& desc);
    private:
        struct RenderPassEq {
            bool operator()(const Blast::GfxRenderPassDesc& desc1, const Blast::GfxRenderPassDesc& desc2) const;
        };
        Renderer* mRenderer = nullptr;
        std::map<Blast::GfxRenderPassDesc, Blast::GfxRenderPass*, RenderPassEq> mRenderPasses;
    };

    class FramebufferCache {
    public:
        FramebufferCache(Renderer* renderer);
        ~FramebufferCache();
        Blast::GfxFramebuffer* getFramebuffer(const Blast::GfxFramebufferDesc& desc);
    private:
        struct FramebufferEq {
            bool operator()(const Blast::GfxFramebufferDesc& desc1, const Blast::GfxFramebufferDesc& desc2) const;
        };
        Renderer* mRenderer = nullptr;
        std::map<Blast::GfxFramebufferDesc, Blast::GfxFramebuffer*, FramebufferEq> mFrambuffers;
    };

    class GraphicsPipelineCache {
    public:
        GraphicsPipelineCache(Renderer* renderer);
        ~GraphicsPipelineCache();
        Blast::GfxGraphicsPipeline* getPipeline(const Blast::GfxGraphicsPipelineDesc& desc);
    private:
        struct PipelineEq {
            bool operator()(const Blast::GfxGraphicsPipelineDesc& desc1, const Blast::GfxGraphicsPipelineDesc& desc2) const;
        };
        Renderer* mRenderer = nullptr;
        std::map<Blast::GfxGraphicsPipelineDesc, Blast::GfxGraphicsPipeline*, PipelineEq> mPipelines;
    };

}