#pragma once
#include "Core/GearDefine.h"
#include "Utility/Hash.h"
#include <Blast/Gfx/GfxDefine.h>
#include <Blast/Gfx/GfxSampler.h>
#include <Blast/Gfx/GfxRenderTarget.h>
#include <Blast/Gfx/GfxPipeline.h>
#include <unordered_map>

namespace gear {
    class Renderer;
    class SamplerCache {
    public:
        SamplerCache(Renderer* renderer);
        ~SamplerCache();
        Blast::GfxSampler* getSampler(const Blast::GfxSamplerDesc& desc);
    private:
        struct SamplerCacheEq {
            bool operator()(const Blast::GfxSamplerDesc& desc1, const Blast::GfxSamplerDesc& desc2) const;
        };
        Renderer* mRenderer = nullptr;
        std::unordered_map<Blast::GfxSamplerDesc, Blast::GfxSampler*, MurmurHash<Blast::GfxSamplerDesc>, SamplerCacheEq> mSamplers;
    };

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
        std::unordered_map<Blast::GfxRenderPassDesc, Blast::GfxRenderPass*, MurmurHash<Blast::GfxRenderPassDesc>, RenderPassEq> mRenderPasses;
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
        std::unordered_map<Blast::GfxFramebufferDesc, Blast::GfxFramebuffer*, MurmurHash<Blast::GfxFramebufferDesc>, FramebufferEq> mFrambuffers;
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
        std::unordered_map<Blast::GfxGraphicsPipelineDesc, Blast::GfxGraphicsPipeline*, MurmurHash<Blast::GfxGraphicsPipelineDesc>, PipelineEq> mPipelines;
    };

    struct DescriptorKey {
        Blast::GfxBuffer* uniformBuffers[3];
        uint32_t uniformBufferOffsets[3];
        uint32_t uniformBufferSizes[3];
        Blast::GfxTexture* textures[8];
        Blast::GfxSampler* samplers[8];
    };

    struct DescriptorBundle {
        Blast::GfxDescriptorSet* handles[2];
    };

    class DescriptorCache {
    public:
        DescriptorCache(Renderer* renderer);
        ~DescriptorCache();
        DescriptorBundle getDescriptor(const DescriptorKey& key);
    private:
        struct DescriptorEq {
            bool operator()(const DescriptorKey& key1, const DescriptorKey& key2) const;
        };
        Renderer* mRenderer = nullptr;
        std::unordered_map<DescriptorKey, DescriptorBundle, MurmurHash<DescriptorKey>, DescriptorEq> mDescriptors;
    };
}