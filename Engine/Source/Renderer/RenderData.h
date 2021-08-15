#pragma once
#include "Core/GearDefine.h"
#include "Math/Math.h"
#include <Blast/Gfx/GfxBuffer.h>
#include <Blast/Gfx/GfxTexture.h>
#include <Blast/Gfx/GfxSampler.h>
#include <Blast/Gfx/GfxShader.h>
#include <Blast/Gfx/GfxCommandBuffer.h>
#include <map>
#include <vector>

namespace gear {
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

    struct FrameUniforms {
        glm::mat4 viewMatrix;
        glm::mat4 projMatrix;
    };

    struct ObjectUniforms {
        glm::mat4 modelMatrix;
        glm::mat4 normalMatrix;
    };


    struct CascadeParameters {
        // clip space的远近平面
        glm::vec2 csNearFar = { -1.0f, 1.0f };

        // light space的远近平面
        glm::vec2 lsNearFar;

        // view space的远近平面
        glm::vec2 vsNearFar;

        // 世界坐标的灯光位置
        glm::vec3 wsLightPosition;

        BBox wsShadowCastersVolume;
        BBox wsShadowReceiversVolume;
    };

    struct FramebufferInfo {
        blast::GfxClearValue clear_value;
        blast::SampleCount sample_count;
        std::tuple<blast::GfxTexture*, uint32_t, uint32_t> colors[TARGET_COUNT];
        std::tuple<blast::GfxTexture*, uint32_t, uint32_t> depth_stencil;
    };

    using DrawCallKey = uint64_t;

    struct DrawCall {
        DrawCallKey key = 0;

        blast::GfxShader* vs = nullptr;
        blast::GfxShader* fs = nullptr;

        blast::GfxBuffer* vb = nullptr;

        uint32_t ib_offset = 0;
        uint32_t ib_count = 0;
        blast::GfxBuffer* ib = nullptr;

        blast::GfxBuffer* bone_ub = nullptr;

        uint32_t renderable_ub_offset = 0;
        blast::GfxBuffer* renderable_ub = nullptr;

        std::pair<blast::GfxBuffer*, blast::GfxSamplerDesc> material_samplers[MATERIAL_SAMPLER_COUNT];
        blast::GfxBuffer* material_ub = nullptr;

        blast::PrimitiveTopology topo;
        RenderState render_state;

        bool operator < (DrawCall const& rhs) { return key < rhs.key; }
    };
}