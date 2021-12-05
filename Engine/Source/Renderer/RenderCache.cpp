#include "RenderCache.h"
#include "Renderer.h"
#include "GearEngine.h"

#include <Blast/Gfx/GfxDevice.h>

namespace gear {

    VertexLayoutCache::VertexLayoutCache() {
        {
            blast::GfxInputLayout* layout = new blast::GfxInputLayout();
            uint32_t offset = 0;
            blast::GfxInputLayout::Element element;
            element.semantic = blast::SEMANTIC_POSITION;
            element.format = blast::FORMAT_R32G32B32_FLOAT;
            element.location = 0;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            layout->elements.push_back(element);
            offset += element.size;

            element.semantic = blast::SEMANTIC_TEXCOORD0;
            element.format = blast::FORMAT_R32G32_FLOAT;
            element.location = 5;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            layout->elements.push_back(element);

            input_layouts[VLT_P_T0] = layout;
        }

        {
            blast::GfxInputLayout* layout = new blast::GfxInputLayout();
            uint32_t offset = 0;
            blast::GfxInputLayout::Element element;
            element.semantic = blast::SEMANTIC_POSITION;
            element.format = blast::FORMAT_R32G32B32_FLOAT;
            element.location = 0;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            layout->elements.push_back(element);
            offset += element.size;

            input_layouts[VLT_P] = layout;
        }

        {
            blast::GfxInputLayout* layout = new blast::GfxInputLayout();
            uint32_t offset = 0;
            blast::GfxInputLayout::Element element;
            element.semantic = blast::SEMANTIC_POSITION;
            element.format = blast::FORMAT_R32G32B32_FLOAT;
            element.location = 0;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            offset += element.size;
            layout->elements.push_back(element);

            element.semantic = blast::SEMANTIC_COLOR;
            element.format = blast::FORMAT_R8G8B8A8_UNORM;
            element.location = 4;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            layout->elements.push_back(element);

            input_layouts[VLT_DEBUG] = layout;
        }

        {
            blast::GfxInputLayout* layout = new blast::GfxInputLayout();
            uint32_t offset = 0;
            blast::GfxInputLayout::Element element;
            element.semantic = blast::SEMANTIC_POSITION;
            element.format = blast::FORMAT_R32G32B32_FLOAT;
            element.location = 0;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            layout->elements.push_back(element);
            offset += element.size;

            element.semantic = blast::SEMANTIC_TEXCOORD0;
            element.format = blast::FORMAT_R32G32_FLOAT;
            element.location = 5;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            layout->elements.push_back(element);
            offset += element.size;

            element.semantic = blast::SEMANTIC_COLOR;
            element.format = blast::FORMAT_R8G8B8A8_UNORM;
            element.location = 4;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            layout->elements.push_back(element);

            input_layouts[VLT_UI] = layout;
        }

        {
            blast::GfxInputLayout* layout = new blast::GfxInputLayout();
            uint32_t offset = 0;
            blast::GfxInputLayout::Element element;
            element.semantic = blast::SEMANTIC_POSITION;
            element.format = blast::FORMAT_R32G32B32_FLOAT;
            element.location = 0;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            layout->elements.push_back(element);
            offset += element.size;

            element.semantic = blast::SEMANTIC_TEXCOORD0;
            element.format = blast::FORMAT_R32G32_FLOAT;
            element.location = 5;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            layout->elements.push_back(element);
            offset += element.size;

            element.semantic = blast::SEMANTIC_NORMAL;
            element.format = blast::FORMAT_R32G32B32_FLOAT;
            element.location = 1;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            layout->elements.push_back(element);
            offset += element.size;

            element.semantic = blast::SEMANTIC_TANGENT;
            element.format = blast::FORMAT_R32G32B32_FLOAT;
            element.location = 2;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            layout->elements.push_back(element);
            offset += element.size;

            element.semantic = blast::SEMANTIC_BITANGENT;
            element.format = blast::FORMAT_R32G32B32_FLOAT;
            element.location = 3;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            layout->elements.push_back(element);

            input_layouts[VLT_STATIC_MESH] = layout;
        }
    }

    VertexLayoutCache::~VertexLayoutCache() {
        for (auto iter = input_layouts.begin(); iter != input_layouts.end(); ++iter) {
            SAFE_DELETE(iter->second);
        }
        input_layouts.clear();
    }

    blast::GfxInputLayout * VertexLayoutCache::GetVertexLayout(VertexLayoutType type) {
        return input_layouts[type];
    }

    RasterizerStateCache::RasterizerStateCache() {
        {
            blast::GfxRasterizerState* rs = new blast::GfxRasterizerState();
            rs->cull_mode = blast::CULL_BACK;
            rasterizer_states[RST_FRONT] = rs;
        }
        {
            blast::GfxRasterizerState* rs = new blast::GfxRasterizerState();
            rs->cull_mode = blast::CULL_FRONT;
            rasterizer_states[RST_BACK] = rs;
        }
        {
            blast::GfxRasterizerState* rs = new blast::GfxRasterizerState();
            rs->cull_mode = blast::CULL_NONE;
            rasterizer_states[RST_DOUBLESIDED] = rs;
        }
    }

    RasterizerStateCache::~RasterizerStateCache() {
        for (auto iter = rasterizer_states.begin(); iter != rasterizer_states.end(); ++iter) {
            SAFE_DELETE(iter->second);
        }
        rasterizer_states.clear();
    }

    blast::GfxRasterizerState * RasterizerStateCache::GetRasterizerState(RasterizerStateType type) {
        return rasterizer_states[type];
    }

    DepthStencilStateCache::DepthStencilStateCache() {
        {
            blast::GfxDepthStencilState* dds = new blast::GfxDepthStencilState();
            dds->depth_test = true;
            dds->depth_write = true;
            dds->stencil_test = true;
            depth_stencil_states[DSST_DEFAULT] = dds;
        }
        {
            blast::GfxDepthStencilState* dds = new blast::GfxDepthStencilState();
            dds->depth_test = true;
            dds->depth_write = true;
            dds->stencil_test = false;
            depth_stencil_states[DSST_SHADOW] = dds;
        }

        {
            blast::GfxDepthStencilState* dds = new blast::GfxDepthStencilState();
            dds->depth_test = false;
            dds->depth_write = true;
            dds->stencil_test = false;
            depth_stencil_states[DSST_UI] = dds;
        }
    }

    DepthStencilStateCache::~DepthStencilStateCache() {
        for (auto iter = depth_stencil_states.begin(); iter != depth_stencil_states.end(); ++iter) {
            SAFE_DELETE(iter->second);
        }
        depth_stencil_states.clear();
    }

    blast::GfxDepthStencilState * DepthStencilStateCache::GetDepthStencilState(DepthStencilStateType type) {
        return depth_stencil_states[type];
    }

    BlendStateCache::BlendStateCache() {
        {
            blast::GfxBlendState* bs = new blast::GfxBlendState();
            bs->rt[0].src_factor = blast::BLEND_ONE;
            bs->rt[0].dst_factor = blast::BLEND_ZERO;
            bs->rt[0].src_factor_alpha = blast::BLEND_ONE;
            bs->rt[0].dst_factor_alpha = blast::BLEND_ZERO;
            bs->rt[0].render_target_write_mask = blast::COLOR_COMPONENT_ALL;
            blend_states[BST_OPAQUE] = bs;
        }
        {
            // 预乘
            blast::GfxBlendState* bs = new blast::GfxBlendState();
            bs->rt[0].src_factor = blast::BLEND_ONE;
            bs->rt[0].dst_factor = blast::BLEND_ONE_MINUS_SRC_ALPHA;
            bs->rt[0].src_factor_alpha = blast::BLEND_ONE;
            bs->rt[0].dst_factor_alpha = blast::BLEND_ONE_MINUS_SRC_ALPHA;
            bs->rt[0].render_target_write_mask = blast::COLOR_COMPONENT_ALL;
            blend_states[BST_TRANSPARENT] = bs;
        }
    }

    BlendStateCache::~BlendStateCache() {
        for (auto iter = blend_states.begin(); iter != blend_states.end(); ++iter) {
            SAFE_DELETE(iter->second);
        }
        blend_states.clear();
    }

    blast::GfxBlendState * BlendStateCache::GetDepthStencilState(BlendStateType type) {
        return blend_states[type];
    }

    bool SamplerCache::SamplerCacheEq::operator()(const blast::GfxSamplerDesc& desc1, const blast::GfxSamplerDesc& desc2) const {
        if (desc1.address_u != desc2.address_u) return false;
        if (desc1.address_v != desc2.address_v) return false;
        if (desc1.address_w != desc2.address_w) return false;
        if (desc1.min_filter != desc2.min_filter) return false;
        if (desc1.mag_filter != desc2.mag_filter) return false;
        if (desc1.mipmap_mode != desc2.mipmap_mode) return false;
        return true;
    }

    SamplerCache::SamplerCache() {
    }

    SamplerCache::~SamplerCache() {
        for (auto iter = samplers.begin(); iter != samplers.end(); ++iter) {
            gEngine.GetDevice()->DestroySampler(iter->second);
        }
        samplers.clear();
    }

    blast::GfxSampler* SamplerCache::GetSampler(const blast::GfxSamplerDesc& desc) {
        auto iter = samplers.find(desc);
        if (iter != samplers.end()) {
            return iter->second;
        }

        blast::GfxSampler* sampler = gEngine.GetDevice()->CreateSampler(desc);
        samplers[desc] = sampler;
        return sampler;
    }

    bool RenderPassCache::RenderPassEq::operator()(const blast::GfxRenderPassDesc& desc1, const blast::GfxRenderPassDesc& desc2) const {
        if (desc1.attachments.size() != desc2.attachments.size()) return false;
        for (uint32_t i = 0; i < desc1.attachments.size(); ++i) {
            if (desc1.attachments[i].type != desc2.attachments[i].type) return false;
            if (desc1.attachments[i].texture != desc2.attachments[i].texture) return false;
            if (desc1.attachments[i].subresource != desc2.attachments[i].subresource) return false;
            if (desc1.attachments[i].loadop != desc2.attachments[i].loadop) return false;
            if (desc1.attachments[i].storeop != desc2.attachments[i].storeop) return false;
        }
        return true;
    }

    RenderPassCache::RenderPassCache() {
    }

    RenderPassCache::~RenderPassCache() {
        for (auto iter = renderpasses.begin(); iter != renderpasses.end(); ++iter) {
            gEngine.GetDevice()->DestroyRenderPass(iter->second);
        }
        renderpasses.clear();
    }

    blast::GfxRenderPass* RenderPassCache::GetRenderPass(const blast::GfxRenderPassDesc& desc) {
        auto iter = renderpasses.find(desc);
        if (iter != renderpasses.end()) {
            return iter->second;
        }

        blast::GfxRenderPass* renderpass = gEngine.GetDevice()->CreateRenderPass(desc);
        renderpasses[desc] = renderpass;
        return renderpass;
    }

    bool PipelineCache::PipelineEq::operator()(const blast::GfxPipelineDesc& desc1, const blast::GfxPipelineDesc& desc2) const {
        if (desc1.vs != desc2.vs) return false;
        if (desc1.fs != desc2.fs) return false;
        if (desc1.hs != desc2.hs) return false;
        if (desc1.ds != desc2.ds) return false;
        if (desc1.gs != desc2.gs) return false;
        if (desc1.bs != desc2.bs) return false;
        if (desc1.rs != desc2.rs) return false;
        if (desc1.dss != desc2.dss) return false;
        if (desc1.il != desc2.il) return false;
        if (desc1.rp != desc2.rp) return false;
        if (desc1.sc != desc2.sc) return false;
        if (desc1.patch_control_points != desc2.patch_control_points) return false;
        if (desc1.sample_count != desc2.sample_count) return false;
        if (desc1.primitive_topo != desc2.primitive_topo) return false;
        return true;
    }

    PipelineCache::PipelineCache() {
    }

    PipelineCache::~PipelineCache() {
        for (auto iter = pipelines.begin(); iter != pipelines.end(); ++iter) {
            gEngine.GetDevice()->DestroyPipeline(iter->second);
        }
        pipelines.clear();
    }

    blast::GfxPipeline* PipelineCache::GetPipeline(const blast::GfxPipelineDesc& desc) {
        auto iter = pipelines.find(desc);
        if (iter != pipelines.end()) {
            return iter->second;
        }

        blast::GfxPipeline* pipeline = gEngine.GetDevice()->CreatePipeline(desc);
        pipelines[desc] = pipeline;
        return pipeline;
    }
}