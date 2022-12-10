#pragma once
#include "Core/GearDefine.h"
#include "Utility/Hash.h"
#include "RenderData.h"

#include <GfxDefine.h>
#include <GfxDevice.h>

#include <unordered_map>

namespace gear {
    class Renderer;

    class VertexLayoutCache {
    public:
        VertexLayoutCache(blast::GfxDevice* in_device);

        ~VertexLayoutCache();

        blast::GfxInputLayout* GetVertexLayout(VertexLayoutType type);

    private:
        blast::GfxDevice* device = nullptr;
        std::unordered_map<VertexLayoutType, std::shared_ptr<blast::GfxInputLayout>> input_layouts;
    };

    class RasterizerStateCache {
    public:
        RasterizerStateCache(blast::GfxDevice* in_device);

        ~RasterizerStateCache();

        blast::GfxRasterizerState* GetRasterizerState(RasterizerStateType type);

    private:
        blast::GfxDevice* device;
        std::unordered_map<RasterizerStateType, std::shared_ptr<blast::GfxRasterizerState>> rasterizer_states;
    };

    class DepthStencilStateCache {
    public:
        DepthStencilStateCache(blast::GfxDevice* in_device);

        ~DepthStencilStateCache();

        blast::GfxDepthStencilState* GetDepthStencilState(DepthStencilStateType type);

    private:
        blast::GfxDevice* device = nullptr;
        std::unordered_map<DepthStencilStateType, std::shared_ptr<blast::GfxDepthStencilState>> depth_stencil_states;
    };

    class BlendStateCache {
    public:
        BlendStateCache(blast::GfxDevice* in_device);

        ~BlendStateCache();

        blast::GfxBlendState* GetDepthStencilState(BlendStateType type);

    private:
        blast::GfxDevice* device = nullptr;
        std::unordered_map<BlendStateType, std::shared_ptr<blast::GfxBlendState>> blend_states;
    };

    class SamplerCache {
    public:
        SamplerCache(blast::GfxDevice* in_device);

        ~SamplerCache();

        blast::GfxSampler* GetSampler(const blast::GfxSamplerDesc& desc);

    private:
        struct SamplerCacheEq {
            bool operator()(const blast::GfxSamplerDesc& desc1, const blast::GfxSamplerDesc& desc2) const;
        };

        blast::GfxDevice* device = nullptr;
        std::unordered_map<blast::GfxSamplerDesc, std::shared_ptr<blast::GfxSampler>, MurmurHash<blast::GfxSamplerDesc>, SamplerCacheEq> samplers;
    };

    class RenderPassCache {
    public:
        RenderPassCache(blast::GfxDevice* in_device);

        ~RenderPassCache();

        blast::GfxRenderPass* GetRenderPass(const blast::GfxRenderPassDesc& desc);

    private:
        struct RenderPassEq {
            bool operator()(const blast::GfxRenderPassDesc& desc1, const blast::GfxRenderPassDesc& desc2) const;
        };

        blast::GfxDevice* device = nullptr;
        std::unordered_map<blast::GfxRenderPassDesc, std::shared_ptr<blast::GfxRenderPass>, MurmurHash<blast::GfxRenderPassDesc>, RenderPassEq> renderpasses;
    };

    class PipelineCache {
    public:
        PipelineCache(blast::GfxDevice* in_device);

        ~PipelineCache();

        blast::GfxPipeline* GetPipeline(const blast::GfxPipelineDesc& desc);

    private:
        struct PipelineEq {
            bool operator()(const blast::GfxPipelineDesc& desc1, const blast::GfxPipelineDesc& desc2) const;
        };

        blast::GfxDevice* device = nullptr;
        std::unordered_map<blast::GfxPipelineDesc, std::shared_ptr<blast::GfxPipeline>, MurmurHash<blast::GfxPipelineDesc>, PipelineEq> pipelines;
    };
}