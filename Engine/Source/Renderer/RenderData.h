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
        blast::CullMode cull_mode = blast::CULL_MODE_NONE;
    };

    struct ViewUniforms {
        glm::mat4 view_matrix;
        glm::mat4 main_view_matrix;
        glm::mat4 proj_matrix;
        glm::mat4 sun_matrixs[SHADOW_CASCADE_COUNT];
        glm::vec4 sun_direction;
        // 级联阴影的划分参数
        glm::vec4 cascade_splits;
    };

    struct RenderableUniforms {
        glm::mat4 model_matrix;
        glm::mat4 normal_matrix;
    };

    struct FramebufferAttachment {
        blast::GfxTexture* texture = nullptr;
        uint32_t level = 0;
        uint32_t num_levels = 1;
        uint32_t layer = 0;
        uint32_t num_layers = 1;
    };

    struct FramebufferInfo {
        bool is_screen_fb = false;
        uint32_t width;
        uint32_t height;
        float viewport[4];
        blast::GfxClearValue clear_value;
        blast::SampleCount sample_count;
        FramebufferAttachment colors[TARGET_COUNT];
        FramebufferAttachment depth_stencil;
    };

    struct SamplerInfo {
        uint32_t slot;
        uint32_t level = 0;
        uint32_t num_levels = 1;
        uint32_t layer = 0;
        uint32_t num_layers = 1;
        blast::GfxTexture* texture;
        blast::GfxSamplerDesc sampler_desc;
    };

    struct UniformDescriptor {
        uint32_t slot;
        blast::GfxBuffer* uniform_buffer;
        uint32_t uniform_buffer_offset;
        uint32_t uniform_buffer_size;
    };

    struct SamplerDescriptor {
        uint32_t slot;
        blast::GfxTextureView* textures_view;
        blast::GfxSampler* sampler;
    };

    struct DescriptorKey {
        uint32_t num_uniform_buffers = 0;
        uint32_t num_samplers = 0;
        UniformDescriptor uniform_descriptors[UBUFFER_BINDING_COUNT];
        SamplerDescriptor sampler_descriptors[MAX_TEXTURE_COUNT];
    };

    struct DrawCall {
        static constexpr uint64_t MATERIAL_INSTANCE_ID_MASK     = 0x00000FFFllu;
        static constexpr unsigned MATERIAL_INSTANCE_ID_SHIFT    = 0;

        static constexpr uint64_t MATERIAL_VARIANT_KEY_MASK     = 0x000FF000llu;
        static constexpr unsigned MATERIAL_VARIANT_KEY_SHIFT    = 12;

        static constexpr uint64_t MATERIAL_ID_MASK              = 0xFFF00000llu;
        static constexpr unsigned MATERIAL_ID_SHIFT             = 20;

        static constexpr uint64_t MATERIAL_MASK                 = 0xFFFFFFFFllu;
        static constexpr unsigned MATERIAL_SHIFT                = 0;

        static uint64_t GetField(uint64_t value, uint64_t mask, unsigned shift) {
            return uint64_t((value & mask)) >> shift;
        }

        static uint64_t GenField(uint64_t value, uint64_t mask, unsigned shift) {
            return uint64_t(value) << shift;
        }

        static uint64_t GenMaterialKey(uint32_t material_id, uint32_t material_variant, uint32_t instance_id) {
            uint64_t key = ((material_id << MATERIAL_ID_SHIFT) & MATERIAL_ID_MASK) |
                           ((material_variant << MATERIAL_VARIANT_KEY_SHIFT) & MATERIAL_VARIANT_KEY_MASK) |
                           ((instance_id << MATERIAL_INSTANCE_ID_SHIFT) & MATERIAL_INSTANCE_ID_MASK);
            return (key << MATERIAL_SHIFT) & MATERIAL_MASK;
        }

        bool operator < (DrawCall const& rhs) { return key < rhs.key; }

        uint64_t key = 0;

        uint32_t material_variant = 0;

        uint32_t renderable_id = 0;

        uint32_t primitive_id = 0;
    };
}