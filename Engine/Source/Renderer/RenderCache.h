#pragma once
#include "Core/GearDefine.h"
#include "Utility/Hash.h"
#include "RenderData.h"

#include <Blast/Gfx/GfxDefine.h>

#include <unordered_map>

namespace gear {
    class Renderer;

    // TODO:定期清理缓存
    // TODO:支持多线程

    class VertexLayoutCache {
    public:
        VertexLayoutCache();

        ~VertexLayoutCache();

        blast::GfxInputLayout* GetVertexLayout(VertexLayoutType type);

    private:
        std::unordered_map<VertexLayoutType, blast::GfxInputLayout*> input_layouts;
    };

    class RasterizerStateCache {
    public:
        RasterizerStateCache();

        ~RasterizerStateCache();

        blast::GfxRasterizerState* GetRasterizerState(RasterizerStateType type);

    private:
        std::unordered_map<RasterizerStateType, blast::GfxRasterizerState*> rasterizer_states;
    };

    class DepthStencilStateCache {
    public:
        DepthStencilStateCache();

        ~DepthStencilStateCache();

        blast::GfxDepthStencilState* GetDepthStencilState(DepthStencilStateType type);

    private:
        std::unordered_map<DepthStencilStateType, blast::GfxDepthStencilState*> depth_stencil_states;
    };

    class BlendStateCache {
    public:
        BlendStateCache();

        ~BlendStateCache();

        blast::GfxBlendState* GetDepthStencilState(BlendStateType type);

    private:
        std::unordered_map<BlendStateType, blast::GfxBlendState*> blend_states;
    };

    class SamplerCache {
    public:
        SamplerCache();

        ~SamplerCache();

        blast::GfxSampler* GetSampler(const blast::GfxSamplerDesc& desc);

    private:
        struct SamplerCacheEq {
            bool operator()(const blast::GfxSamplerDesc& desc1, const blast::GfxSamplerDesc& desc2) const;
        };

        std::unordered_map<blast::GfxSamplerDesc, blast::GfxSampler*, MurmurHash<blast::GfxSamplerDesc>, SamplerCacheEq> samplers;
    };

    class RenderPassCache {
    public:
        RenderPassCache();

        ~RenderPassCache();

        blast::GfxRenderPass* GetRenderPass(const blast::GfxRenderPassDesc& desc);

    private:
        struct RenderPassEq {
            bool operator()(const blast::GfxRenderPassDesc& desc1, const blast::GfxRenderPassDesc& desc2) const;
        };

        std::unordered_map<blast::GfxRenderPassDesc, blast::GfxRenderPass*, MurmurHash<blast::GfxRenderPassDesc>, RenderPassEq> renderpasses;
    };

    class PipelineCache {
    public:
        PipelineCache();

        ~PipelineCache();

        blast::GfxPipeline* GetPipeline(const blast::GfxPipelineDesc& desc);

    private:
        struct PipelineEq {
            bool operator()(const blast::GfxPipelineDesc& desc1, const blast::GfxPipelineDesc& desc2) const;
        };

        std::unordered_map<blast::GfxPipelineDesc, blast::GfxPipeline*, MurmurHash<blast::GfxPipelineDesc>, PipelineEq> pipelines;
    };
}