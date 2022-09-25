#pragma once
#include "Core/GearDefine.h"
#include "Math/Math.h"
#include "Math/Geometry.h"

#include <Blast/Gfx/GfxDefine.h>

#include <map>
#include <vector>

namespace gear {
    class VertexBuffer;
    class IndexBuffer;
    class UniformBuffer;
    class MaterialInstance;

    enum ShaderDomain {
        SHADER_GLOBAL = 0,
        SHADER_SURFACE = 1
    };

    // 着色模型
    enum ShadingModel {
        SHADING_MODEL_GLOBAL,
        SHADING_MODEL_UNLIT,
        SHADING_MODEL_LIT,
    };

    // P表示位置,T0表示纹理坐标
    enum VertexLayoutType {
        VLT_P = 0,
        VLT_P_T0,
        VLT_DEBUG,
        VLT_UI,
        VLT_STATIC_MESH,
        VLT_SKIN_MESH,
        VLT_COUNT
    };

    enum RasterizerStateType {
        RST_FRONT,
        RST_BACK,
        RST_DOUBLESIDED,
        RST_COUNT
    };

    enum DepthStencilStateType {
        DSST_DEFAULT,
        DSST_SHADOW,
        DSST_UI,
        DSST_COUNT
    };

    enum BlendStateType {
        BST_OPAQUE,
        BST_TRANSPARENT,
        BST_COUNT
    };

    struct ViewUniforms {
        glm::mat4 view_matrix;
        glm::mat4 main_view_matrix;
        glm::mat4 proj_matrix;
        glm::mat4 sun_matrixs[SHADOW_CASCADE_COUNT];
        glm::vec4 sun_direction;
        glm::vec4 sun_color_intensity;
        glm::vec4 view_position;
        // 级联阴影的划分参数
        glm::vec4 cascade_splits;
        float ev100;
        float exposure;
    };

    struct RenderableUniforms {
        glm::mat4 local_matrix;
        glm::mat4 model_matrix;
        glm::mat4 normal_matrix;
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

    struct LightInfo {
        bool has_direction_light = false;
        glm::vec3 sun_direction;
        glm::vec4 sun_color_intensity;
        bool has_ibl = false;
        blast::GfxTexture* irradiance_map = nullptr;
        blast::GfxTexture* prefiltered_map = nullptr;
        blast::GfxTexture* lut = nullptr;
    };

    struct CameraInfo {
        float zn;
        float zf;
        float ev100;
        float exposure;
        glm::vec3 position;
		glm::vec3 view_direction;
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };

    struct ShadowMapInfo {
        // clip space的远近平面
        glm::vec2 cs_near_far;
        glm::mat4 light_view_matrix;
        glm::mat4 light_projection_matrix;
        glm::vec3 camera_position;
        glm::vec3 camera_direction;
        // 阴影贴图原始分辨率
        uint32_t texture_dimension = 0;
        // 实际使用的阴影贴图分辨率，牺牲最外围像素做边界
        uint32_t shadow_dimension = 0;
    };

	struct AtmosphereParameters {
		glm::vec4 rayleigh_scattering;
		glm::vec4 mie_scattering;
		glm::vec4 mie_extinction;
		glm::vec4 mie_absorption;
		glm::vec4 absorption_extinction;
		glm::vec4 ground_albedo;
		glm::vec4 sun_direction;
		glm::vec4 view_direction;
		float bottom_radius;
		float top_radius;
		float rayleigh_density_exp_scale;
		float mie_density_exp_scale;
		float mie_phase_g;
		float absorption_density0_layer_width;
		float absorption_density0_constant_term;
		float absorption_density0_linear_term;
		float absorption_density1_constant_term;
		float absorption_density1_linear_term;
	};

    enum RenderableType {
        RENDERABLE_COMMON = 0,
        RENDERABLE_UI = 1
    };

    struct UIDrawElement {
        uint32_t count = 0;
        uint32_t offset = 0;
        MaterialInstance* mi = nullptr;
        VertexBuffer* vb = nullptr;
        IndexBuffer* ib = nullptr;
    };

    struct RenderPrimitive {
        bool cast_shadow = false;
        bool receive_shadow = true;
        uint32_t count = 0;
        uint32_t offset = 0;
        BBox bbox;
        MaterialInstance* mi = nullptr;
        VertexBuffer* vb = nullptr;;
        IndexBuffer* ib = nullptr;;
        UniformBuffer* material_ub = nullptr;
        blast::PrimitiveTopology topo = blast::PrimitiveTopology::PRIMITIVE_TOPO_TRI_LIST;
    };

    struct Renderable {
        uint32_t renderable_ub_size;
        uint32_t renderable_ub_offset;
        blast::GfxBuffer* renderable_ub = nullptr;
        blast::GfxBuffer* bone_ub = nullptr;
        uint32_t num_primitives = 0;
        RenderPrimitive primitives[MAX_RENDER_PRIMITIVE_COUNT];
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

    uint32_t GetVertexLayoutStride(VertexLayoutType vertex_layout);
}