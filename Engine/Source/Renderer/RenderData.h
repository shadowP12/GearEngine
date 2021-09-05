#pragma once
#include "Core/GearDefine.h"
#include "Math/Math.h"
#include "Math/Geometry.h"
#include <Blast/Gfx/GfxPipeline.h>
#include <Blast/Gfx/GfxBuffer.h>
#include <Blast/Gfx/GfxTexture.h>
#include <Blast/Gfx/GfxSampler.h>
#include <Blast/Gfx/GfxShader.h>
#include <Blast/Gfx/GfxCommandBuffer.h>
#include <map>
#include <vector>

namespace gear {
    // Shader应用的范围
    enum ShaderDomain {
        SHADER_GLOBAL = 0,
        SHADER_SURFACE = 1
    };

    // 着色模型
    enum ShadingModel {
        SHADING_MODEL_UNLIT,
        SHADING_MODEL_LIT,
    };

    // 混合模式
    enum BlendingMode {
        BLENDING_MODE_OPAQUE,
        BLENDING_MODE_TRANSPARENT,
        BLENDING_MODE_MASKED,
    };

    struct RenderState {
        ShadingModel shading_model;
        BlendingMode blending_mode;
    };

    struct FramebufferInfo {
        bool is_screen_fb = false;
        uint32_t width;
        uint32_t height;
        blast::GfxClearValue clear_value;
        blast::SampleCount sample_count;
        // texture/layer/level
        std::tuple<blast::GfxTexture*, uint32_t, uint32_t> colors[TARGET_COUNT];
        std::tuple<blast::GfxTexture*, uint32_t, uint32_t> depth_stencil;
    };

    using DrawCallKey = uint64_t;

    struct DrawCall {
        DrawCallKey key = 0;

        blast::GfxShader* vs = nullptr;
        blast::GfxShader* fs = nullptr;

        uint32_t vb_offset = 0;
        uint32_t vb_count = 0;
        blast::GfxVertexLayout vertex_layout;
        blast::GfxBuffer* vb = nullptr;

        uint32_t ib_offset = 0;
        uint32_t ib_count = 0;
        blast::IndexType ib_type;
        blast::GfxBuffer* ib = nullptr;

        blast::GfxBuffer* bone_ub = nullptr;

        uint32_t renderable_ub_size = 0;
        uint32_t renderable_ub_offset = 0;
        blast::GfxBuffer* renderable_ub = nullptr;

        std::pair<blast::GfxTexture*, blast::GfxSamplerDesc> material_samplers[MATERIAL_SAMPLER_COUNT];

        uint32_t material_ub_size = 0;
        uint32_t material_ub_offset = 0;
        blast::GfxBuffer* material_ub = nullptr;

        blast::PrimitiveTopology topo;
        RenderState render_state;

        bool operator < (DrawCall const& rhs) { return key < rhs.key; }
    };
}