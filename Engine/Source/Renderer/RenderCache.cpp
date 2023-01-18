#include "RenderCache.h"
#include "Renderer.h"
#include "GearEngine.h"

namespace gear {

    VertexLayoutCache::VertexLayoutCache(blast::GfxDevice* in_device) {
        device = in_device;
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
            layout->elements.push_back(element);\
            offset += element.size;

            input_layouts[VLT_P_T0] = std::shared_ptr<blast::GfxInputLayout>(layout);
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

            input_layouts[VLT_P] = std::shared_ptr<blast::GfxInputLayout>(layout);
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

            element.semantic = blast::SEMANTIC_COLOR;
            element.format = blast::FORMAT_R32G32B32A32_FLOAT;
            element.location = 4;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            layout->elements.push_back(element);
            offset += element.size;

            input_layouts[VLT_DEBUG] = std::shared_ptr<blast::GfxInputLayout>(layout);
        }

        {
            blast::GfxInputLayout* layout = new blast::GfxInputLayout();
            uint32_t offset = 0;
            blast::GfxInputLayout::Element element;
            element.semantic = blast::SEMANTIC_POSITION;
            element.format = blast::FORMAT_R32G32_FLOAT;
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
            offset += element.size;

            input_layouts[VLT_UI] = std::shared_ptr<blast::GfxInputLayout>(layout);
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
            offset += element.size;

            input_layouts[VLT_STATIC_MESH] = std::shared_ptr<blast::GfxInputLayout>(layout);
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
            offset += element.size;

            element.semantic = blast::SEMANTIC_JOINTS;
            element.format = blast::FORMAT_R16G16B16A16_UINT;
            element.location = 7;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            layout->elements.push_back(element);
            offset += element.size;

            element.semantic = blast::SEMANTIC_WEIGHTS;
            element.format = blast::FORMAT_R32G32B32A32_FLOAT;
            element.location = 8;
            element.offset = offset;
            element.size = blast::GetFormatStride(element.format);
            layout->elements.push_back(element);
            offset += element.size;

            input_layouts[VLT_SKIN_MESH] = std::shared_ptr<blast::GfxInputLayout>(layout);
        }
    }

    VertexLayoutCache::~VertexLayoutCache() {
        input_layouts.clear();
    }

    blast::GfxInputLayout * VertexLayoutCache::GetVertexLayout(VertexLayoutType type) {
        return input_layouts[type].get();
    }

    RasterizerStateCache::RasterizerStateCache(blast::GfxDevice* in_device) {
        device = in_device;
        {
            blast::GfxRasterizerState* rs = new blast::GfxRasterizerState();
            rs->cull_mode = blast::CULL_BACK;
            rasterizer_states[RST_FRONT] = std::shared_ptr<blast::GfxRasterizerState>(rs);
        }
        {
            blast::GfxRasterizerState* rs = new blast::GfxRasterizerState();
            rs->cull_mode = blast::CULL_FRONT;
            rasterizer_states[RST_BACK] = std::shared_ptr<blast::GfxRasterizerState>(rs);
        }
        {
            blast::GfxRasterizerState* rs = new blast::GfxRasterizerState();
            rs->cull_mode = blast::CULL_NONE;
            rasterizer_states[RST_DOUBLESIDED] = std::shared_ptr<blast::GfxRasterizerState>(rs);
        }
    }

    RasterizerStateCache::~RasterizerStateCache() {
        rasterizer_states.clear();
    }

    blast::GfxRasterizerState * RasterizerStateCache::GetRasterizerState(RasterizerStateType type) {
        return rasterizer_states[type].get();
    }

    DepthStencilStateCache::DepthStencilStateCache(blast::GfxDevice* in_device) {
        device = in_device;
        {
            blast::GfxDepthStencilState* dds = new blast::GfxDepthStencilState();
            dds->depth_test = true;
            dds->depth_write = true;
            dds->stencil_test = true;
            depth_stencil_states[DSST_DEFAULT] = std::shared_ptr<blast::GfxDepthStencilState>(dds);
        }
        {
            blast::GfxDepthStencilState* dds = new blast::GfxDepthStencilState();
            dds->depth_test = true;
            dds->depth_write = true;
            dds->stencil_test = false;
            depth_stencil_states[DSST_SHADOW] = std::shared_ptr<blast::GfxDepthStencilState>(dds);
        }

        {
            blast::GfxDepthStencilState* dds = new blast::GfxDepthStencilState();
            dds->depth_test = false;
            dds->depth_write = true;
            dds->stencil_test = false;
            depth_stencil_states[DSST_UI] = std::shared_ptr<blast::GfxDepthStencilState>(dds);
        }
    }

    DepthStencilStateCache::~DepthStencilStateCache() {
        depth_stencil_states.clear();
    }

    blast::GfxDepthStencilState * DepthStencilStateCache::GetDepthStencilState(DepthStencilStateType type) {
        return depth_stencil_states[type].get();
    }

    BlendStateCache::BlendStateCache(blast::GfxDevice* in_device) {
        device = in_device;
        {
            blast::GfxBlendState* bs = new blast::GfxBlendState();
            bs->rt[0].blend_enable = false;
            bs->rt[0].src_factor = blast::BLEND_ONE;
            bs->rt[0].dst_factor = blast::BLEND_ZERO;
            bs->rt[0].src_factor_alpha = blast::BLEND_ONE;
            bs->rt[0].dst_factor_alpha = blast::BLEND_ZERO;
            bs->rt[0].render_target_write_mask = blast::COLOR_COMPONENT_ALL;
            blend_states[BST_OPAQUE] = std::shared_ptr<blast::GfxBlendState>(bs);
        }
        {
            // 预乘
            blast::GfxBlendState* bs = new blast::GfxBlendState();
            bs->rt[0].blend_enable = true;
            bs->rt[0].src_factor = blast::BLEND_ONE;
            bs->rt[0].dst_factor = blast::BLEND_ONE_MINUS_SRC_ALPHA;
            bs->rt[0].src_factor_alpha = blast::BLEND_ONE;
            bs->rt[0].dst_factor_alpha = blast::BLEND_ONE_MINUS_SRC_ALPHA;
            bs->rt[0].render_target_write_mask = blast::COLOR_COMPONENT_ALL;
            blend_states[BST_TRANSPARENT] = std::shared_ptr<blast::GfxBlendState>(bs);
        }
    }

    BlendStateCache::~BlendStateCache() {
        blend_states.clear();
    }

    blast::GfxBlendState* BlendStateCache::GetBlendState(BlendStateType type) {
        return blend_states[type].get();
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

    SamplerCache::SamplerCache(blast::GfxDevice* in_device) {
        device = in_device;
    }

    SamplerCache::~SamplerCache() {
        samplers.clear();
    }

    blast::GfxSampler* SamplerCache::GetSampler(const blast::GfxSamplerDesc& desc) {
        auto iter = samplers.find(desc);
        if (iter != samplers.end()) {
            return iter->second.get();
        }

        blast::GfxSampler* sampler = device->CreateSampler(desc);
        samplers[desc] = std::shared_ptr<blast::GfxSampler>(sampler);
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

    RenderPassCache::RenderPassCache(blast::GfxDevice* in_device) {
        device = in_device;
    }

    RenderPassCache::~RenderPassCache() {
        renderpasses.clear();
    }

    blast::GfxRenderPass* RenderPassCache::GetRenderPass(const blast::GfxRenderPassDesc& desc) {
        auto iter = renderpasses.find(desc);
        if (iter != renderpasses.end()) {
            return iter->second.get();
        }

        blast::GfxRenderPass* renderpass = device->CreateRenderPass(desc);
        renderpasses[desc] = std::shared_ptr<blast::GfxRenderPass>(renderpass);
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

    PipelineCache::PipelineCache(blast::GfxDevice* in_device) {
        device = in_device;
    }

    PipelineCache::~PipelineCache() {
        pipelines.clear();
    }

    blast::GfxPipeline* PipelineCache::GetPipeline(const blast::GfxPipelineDesc& desc) {
        auto iter = pipelines.find(desc);
        if (iter != pipelines.end()) {
            return iter->second.get();
        }

        blast::GfxPipeline* pipeline = device->CreatePipeline(desc);
        pipelines[desc] = std::shared_ptr<blast::GfxPipeline>(pipeline);
        return pipeline;
    }
}