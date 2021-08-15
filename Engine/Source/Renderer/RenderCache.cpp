#include "RenderCache.h"
#include "Renderer.h"
#include "RenderBuiltinResource.h"
#include <Blast/Gfx/GfxContext.h>

namespace gear {
    bool SamplerCache::SamplerCacheEq::operator()(const blast::GfxSamplerDesc& desc1, const blast::GfxSamplerDesc& desc2) const {
        if (desc1.address_u != desc2.address_u) return false;
        if (desc1.address_v != desc2.address_v) return false;
        if (desc1.address_w != desc2.address_w) return false;
        if (desc1.min_filter != desc2.min_filter) return false;
        if (desc1.mag_filter != desc2.mag_filter) return false;
        if (desc1.mipmap_mode != desc2.mipmap_mode) return false;
        return true;
    }

    SamplerCache::SamplerCache(Renderer* renderer) {
        _renderer = renderer;
    }

    SamplerCache::~SamplerCache() {
        for (auto iter = _samplers.begin(); iter != _samplers.end(); ++iter) {
            _renderer->GetContext()->DestroySampler(iter->second);
        }
        _samplers.clear();
    }

    blast::GfxSampler* SamplerCache::GetSampler(const blast::GfxSamplerDesc& desc) {
        auto iter = _samplers.find(desc);
        if (iter != _samplers.end()) {
            return iter->second;
        }

        blast::GfxSampler* sampler = _renderer->GetContext()->CreateSampler(desc);
        _samplers[desc] = sampler;
        return sampler;
    }

    bool TextureViewCache::TextureViewCacheEq::operator()(const blast::GfxTextureViewDesc& desc1, const blast::GfxTextureViewDesc& desc2) const {
        if (desc1.level != desc2.level) return false;
        if (desc1.layer != desc2.layer) return false;
        if (desc1.texture != desc2.texture) return false;
        return true;
    }

    TextureViewCache::TextureViewCache(Renderer* renderer) {
        _renderer = renderer;
    }

    TextureViewCache::~TextureViewCache() {
        for (auto iter = _texture_views.begin(); iter != _texture_views.end(); ++iter) {
            _renderer->GetContext()->DestroyTextureView(iter->second);
        }
        _texture_views.clear();
    }

    blast::GfxTextureView* TextureViewCache::GetTextureView(const blast::GfxTextureViewDesc& desc) {
        auto iter = _texture_views.find(desc);
        if (iter != _texture_views.end()) {
            return iter->second;
        }

        blast::GfxTextureView* texture_view = _renderer->GetContext()->CreateTextureView(desc);
        _texture_views[desc] = texture_view;
        return texture_view;
    }

    bool FramebufferCache::FramebufferEq::operator()(const blast::GfxFramebufferDesc& desc1, const blast::GfxFramebufferDesc& desc2) const {
        if (desc1.width != desc2.width) return false;
        if (desc1.height != desc2.height) return false;
        if (desc1.sample_count != desc2.sample_count) return false;
        if (desc1.has_depth_stencil != desc2.has_depth_stencil) return false;
        if (desc1.num_colors != desc2.num_colors) return false;
        if (desc1.depth_stencil != desc2.depth_stencil) return false;
        for (int i = 0; i < MAX_RENDER_TARGET_ATTACHMENTS; i++) {
            if (desc1.colors[i] != desc2.colors[i]) return false;
        }
        return true;
    }

    FramebufferCache::FramebufferCache(Renderer* renderer) {
        _renderer = renderer;
    }

    FramebufferCache::~FramebufferCache() {
        for (auto iter = _framebuffers.begin(); iter != _framebuffers.end(); ++iter) {
            _renderer->GetContext()->DestroyFramebuffer(iter->second);
        }
        _framebuffers.clear();
    }

    blast::GfxFramebuffer* FramebufferCache::GetFramebuffer(const blast::GfxFramebufferDesc& desc) {
        auto iter = _framebuffers.find(desc);
        if (iter != _framebuffers.end()) {
            return iter->second;
        }

        blast::GfxFramebuffer* framebuffer = _renderer->GetContext()->CreateFramebuffer(desc);
        _framebuffers[desc] = framebuffer;
        return framebuffer;
    }

    bool GraphicsPipelineCache::PipelineEq::operator()(const blast::GfxGraphicsPipelineDesc& desc1, const blast::GfxGraphicsPipelineDesc& desc2) const {
        if (desc1.framebuffer != desc2.framebuffer) return false;

        if (desc1.vertex_shader != desc2.vertex_shader) return false;
        if (desc1.hull_shader != desc2.hull_shader) return false;
        if (desc1.domain_shader != desc2.domain_shader) return false;
        if (desc1.geometry_shader != desc2.geometry_shader) return false;
        if (desc1.pixel_shader != desc2.pixel_shader) return false;
        if (desc1.root_signature != desc2.root_signature) return false;

        // Vertex Layout
        if (desc1.vertex_layout.num_attributes != desc2.vertex_layout.num_attributes) return false;
        for (int i = 0; i < MAX_VERTEX_ATTRIBS; i++) {
            if (desc1.vertex_layout.attributes[i].format != desc2.vertex_layout.attributes[i].format) return false;
            if (desc1.vertex_layout.attributes[i].semantic != desc2.vertex_layout.attributes[i].semantic) return false;
            if (desc1.vertex_layout.attributes[i].rate != desc2.vertex_layout.attributes[i].rate) return false;
            if (desc1.vertex_layout.attributes[i].location != desc2.vertex_layout.attributes[i].location) return false;
            if (desc1.vertex_layout.attributes[i].binding != desc2.vertex_layout.attributes[i].binding) return false;
            if (desc1.vertex_layout.attributes[i].offset != desc2.vertex_layout.attributes[i].offset) return false;
        }

        // Blend State
        if (desc1.blend_state.independent_blend != desc2.blend_state.independent_blend) return false;
        if (desc1.blend_state.target_mask != desc2.blend_state.target_mask) return false;
        for (int i = 0; i < MAX_RENDER_TARGET_ATTACHMENTS; i++) {
            if (desc1.blend_state.blend_alpha_ops[i] != desc2.blend_state.blend_alpha_ops[i]) return false;
            if (desc1.blend_state.blend_ops[i] != desc2.blend_state.blend_ops[i]) return false;
            if (desc1.blend_state.src_alpha_factors[i] != desc2.blend_state.src_alpha_factors[i]) return false;
            if (desc1.blend_state.dst_alpha_factors[i] != desc2.blend_state.dst_alpha_factors[i]) return false;
            if (desc1.blend_state.src_factors[i] != desc2.blend_state.src_factors[i]) return false;
            if (desc1.blend_state.dst_factors[i] != desc2.blend_state.dst_factors[i]) return false;
            if (desc1.blend_state.color_write_masks[i] != desc2.blend_state.color_write_masks[i]) return false;
        }

        // Depth State
        if (desc1.depth_state.depth_test != desc2.depth_state.depth_test) return false;
        if (desc1.depth_state.depth_write != desc2.depth_state.depth_write) return false;
        if (desc1.depth_state.depth_func != desc2.depth_state.depth_func) return false;
        if (desc1.depth_state.stencil_test != desc2.depth_state.stencil_test) return false;
        if (desc1.depth_state.stencil_read_mask != desc2.depth_state.stencil_read_mask) return false;
        if (desc1.depth_state.stencil_write_mask != desc2.depth_state.stencil_write_mask) return false;
        if (desc1.depth_state.stencil_front_func != desc2.depth_state.stencil_front_func) return false;
        if (desc1.depth_state.stencil_front_fail != desc2.depth_state.stencil_front_fail) return false;
        if (desc1.depth_state.depth_front_fail != desc2.depth_state.depth_front_fail) return false;
        if (desc1.depth_state.stencil_front_pass != desc2.depth_state.stencil_front_pass) return false;
        if (desc1.depth_state.stencil_back_func != desc2.depth_state.stencil_back_func) return false;
        if (desc1.depth_state.stencil_back_fail != desc2.depth_state.stencil_back_fail) return false;
        if (desc1.depth_state.depth_back_fail != desc2.depth_state.depth_back_fail) return false;
        if (desc1.depth_state.stencil_back_pass != desc2.depth_state.stencil_back_pass) return false;

        // Rasterizer State
        if (desc1.rasterizer_state.depth_bias != desc2.rasterizer_state.depth_bias) return false;
        if (desc1.rasterizer_state.slope_scaled_depth_bias != desc2.rasterizer_state.slope_scaled_depth_bias) return false;
        if (desc1.rasterizer_state.depth_clamp_enable != desc2.rasterizer_state.depth_clamp_enable) return false;
        if (desc1.rasterizer_state.primitive_topo != desc2.rasterizer_state.primitive_topo) return false;
        if (desc1.rasterizer_state.fill_mode != desc2.rasterizer_state.fill_mode) return false;
        if (desc1.rasterizer_state.front_face != desc2.rasterizer_state.front_face) return false;
        if (desc1.rasterizer_state.cull_mode != desc2.rasterizer_state.cull_mode) return false;

        return true;
    }

    GraphicsPipelineCache::GraphicsPipelineCache(Renderer* renderer) {
        _renderer = renderer;
    }

    GraphicsPipelineCache::~GraphicsPipelineCache() {
        for (auto iter = _pipelines.begin(); iter != _pipelines.end(); ++iter) {
            _renderer->GetContext()->DestroyGraphicsPipeline(iter->second);
        }
        _pipelines.clear();
    }

    blast::GfxGraphicsPipeline* GraphicsPipelineCache::GetGraphicsPipeline(const blast::GfxGraphicsPipelineDesc& desc) {
        auto iter = _pipelines.find(desc);
        if (iter != _pipelines.end()) {
            return iter->second;
        }

        blast::GfxGraphicsPipeline* pipeline = _renderer->GetContext()->CreateGraphicsPipeline(desc);
        _pipelines[desc] = pipeline;
        return pipeline;
    }

    bool DescriptorCache::DescriptorEq::operator()(const DescriptorKey& key1, const DescriptorKey& key2) const {
        for (int i = 0; i < UBUFFER_BINDING_COUNT; ++i) {
            if (key1.uniform_buffers[i] != key2.uniform_buffers[i]) return false;
            if (key1.uniform_buffer_offsets[i] != key2.uniform_buffer_offsets[i]) return false;
            if (key1.uniform_buffer_sizes[i] != key2.uniform_buffer_sizes[i]) return false;
        }

        for (int i = 0; i < SAMPLER_BINDING_COUNT; ++i) {
            if (key1.textures_views[i] != key2.textures_views[i]) return false;
            if (key1.samplers[i] != key2.samplers[i]) return false;
        }
        return true;
    }

    DescriptorCache::DescriptorCache(Renderer* renderer) {
        _renderer = renderer;
    }

    DescriptorCache::~DescriptorCache() {
        for (auto iter = _descriptors.begin(); iter != _descriptors.end(); ++iter) {
            SAFE_DELETE(iter->second.handles[0]);
            SAFE_DELETE(iter->second.handles[1]);
        }
        _descriptors.clear();
    }

    DescriptorBundle DescriptorCache::GetDescriptor(const DescriptorKey& key) {
        auto iter = _descriptors.find(key);
        if (iter != _descriptors.end()) {
            return iter->second;
        }

        DescriptorBundle bundle;
        bundle.handles[0] = _renderer->GetRootSignature()->allocateSet(0);
        bundle.handles[1] = _renderer->GetRootSignature()->allocateSet(1);

        for (int i = 0; i < UBUFFER_BINDING_COUNT; ++i) {
            if (key.uniform_buffers[i] != nullptr) {
                bundle.handles[0]->SetUniformBuffer(i, key.uniform_buffers[i], key.uniform_buffer_sizes[i], key.uniform_buffer_offsets[i]);
            }
        }

        for (int i = 0; i < SAMPLER_BINDING_COUNT; ++i) {
            if (key.textures_views[i] != nullptr) {
                bundle.handles[1]->SetCombinedSampler(i, key.textures_views[i], key.samplers[i]);
            }
        }

        _descriptors[key] = bundle;
        return bundle;
    }
}