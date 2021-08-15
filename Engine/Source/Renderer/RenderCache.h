#pragma once
#include "Core/GearDefine.h"
#include "Utility/Hash.h"
#include <Blast/Gfx/GfxDefine.h>
#include <Blast/Gfx/GfxSampler.h>
#include <Blast/Gfx/GfxTexture.h>
#include <Blast/Gfx/GfxFramebuffer.h>
#include <Blast/Gfx/GfxPipeline.h>
#include <unordered_map>

namespace gear {
    class Renderer;
    class SamplerCache {
    public:
        SamplerCache(Renderer* renderer);

        ~SamplerCache();

        blast::GfxSampler* GetSampler(const blast::GfxSamplerDesc& desc);

    private:
        struct SamplerCacheEq {
            bool operator()(const blast::GfxSamplerDesc& desc1, const blast::GfxSamplerDesc& desc2) const;
        };

        Renderer* _renderer = nullptr;
        std::unordered_map<blast::GfxSamplerDesc, blast::GfxSampler*, MurmurHash<blast::GfxSamplerDesc>, SamplerCacheEq> _samplers;
    };

    class TextureViewCache {
    public:
        TextureViewCache(Renderer* renderer);

        ~TextureViewCache();

        blast::GfxTextureView* GetTextureView(const blast::GfxTextureViewDesc& desc);
        
    private:
        struct TextureViewCacheEq {
            bool operator()(const blast::GfxTextureViewDesc& desc1, const blast::GfxTextureViewDesc& desc2) const;
        };

        Renderer* _renderer = nullptr;
        std::unordered_map<blast::GfxTextureViewDesc, blast::GfxTextureView*, MurmurHash<blast::GfxTextureViewDesc>, TextureViewCacheEq> _texture_views;
    };

    class FramebufferCache {
    public:
        FramebufferCache(Renderer* renderer);

        ~FramebufferCache();

        blast::GfxFramebuffer* GetFramebuffer(const blast::GfxFramebufferDesc& desc);

    private:
        struct FramebufferEq {
            bool operator()(const blast::GfxFramebufferDesc& desc1, const blast::GfxFramebufferDesc& desc2) const;
        };

        Renderer* _renderer = nullptr;
        std::unordered_map<blast::GfxFramebufferDesc, blast::GfxFramebuffer*, MurmurHash<blast::GfxFramebufferDesc>, FramebufferEq> _framebuffers;
    };

    class GraphicsPipelineCache {
    public:
        GraphicsPipelineCache(Renderer* renderer);

        ~GraphicsPipelineCache();

        blast::GfxGraphicsPipeline* GetGraphicsPipeline(const blast::GfxGraphicsPipelineDesc& desc);

    private:
        struct PipelineEq {
            bool operator()(const blast::GfxGraphicsPipelineDesc& desc1, const blast::GfxGraphicsPipelineDesc& desc2) const;
        };

        Renderer* _renderer = nullptr;
        std::unordered_map<blast::GfxGraphicsPipelineDesc, blast::GfxGraphicsPipeline*, MurmurHash<blast::GfxGraphicsPipelineDesc>, PipelineEq> _pipelines;
    };

    struct DescriptorKey {
        blast::GfxBuffer* uniform_buffers[3];
        uint32_t uniform_buffer_offsets[3];
        uint32_t uniform_buffer_sizes[3];
        blast::GfxTextureView* textures_views[8];
        blast::GfxSampler* samplers[8];
    };

    struct DescriptorBundle {
        blast::GfxDescriptorSet* handles[2];
    };

    class DescriptorCache {
    public:
        DescriptorCache(Renderer* renderer);

        ~DescriptorCache();

        DescriptorBundle GetDescriptor(const DescriptorKey& key);

    private:
        struct DescriptorEq {
            bool operator()(const DescriptorKey& key1, const DescriptorKey& key2) const;
        };

        Renderer* _renderer = nullptr;
        std::unordered_map<DescriptorKey, DescriptorBundle, MurmurHash<DescriptorKey>, DescriptorEq> _descriptors;
    };
}