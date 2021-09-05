#pragma once
#include "Renderer/RenderData.h"
#include "Utility/Flags.h"

#define MAX_DEBUG_POINTS 2048
#define MAX_DEBUG_LINES 2048

namespace gear {
    class Renderer;
    class Scene;
    class Texture;
    class IndexBuffer;
    class VertexBuffer;
    class UniformBuffer;
    class MaterialInstance;

    struct LightInfo {
        glm::vec3 sun_direction;
    };

    struct CameraInfo {
        float zn;
        float zf;
        glm::vec3 position;
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };

    struct CascadeParameters {
        // clip space的远近平面
        glm::vec2 cs_near_far = { -1.0f, 1.0f };

        // light space的远近平面
        glm::vec2 ls_near_far;

        // view space的远近平面
        glm::vec2 vs_near_far;

        // 世界坐标的灯光位置
        glm::vec3 ws_light_position;

        BBox ws_shadow_casters_volume;
        BBox ws_shadow_receivers_volume;
    };

    struct ShadowMapInfo {
        glm::mat4 light_view_matrix;
        glm::mat4 light_projection_matrix;
        glm::vec3 camera_position;
        glm::vec3 camera_direction;
        bool has_visible_shadows;
    };

    struct ViewUniforms {
        glm::mat4 view_matrix;
        glm::mat4 proj_matrix;
    };

    struct RenderableUniforms {
        glm::mat4 model_matrix;
        glm::mat4 normal_matrix;
    };

    struct RenderLight {

    };

    struct RenderView {

    };

    //
    enum RenderableType {
        RENDERABLE_COMMON = 0,
        RENDERABLE_UI = 1
    };

    struct RenderPrimitive {
        bool cast_shadow = false;
        bool receive_shadow = true;
        uint32_t offset = 0;
        uint32_t count = 0;
        BBox bbox;
        MaterialInstance* mi = nullptr;
        VertexBuffer* vb = nullptr;
        IndexBuffer* ib = nullptr;
        blast::PrimitiveTopology topo = blast::PrimitiveTopology::PRIMITIVE_TOPO_TRI_STRIP;
    };

    struct Renderable {
        uint32_t renderable_ub_size;
        uint32_t renderable_ub_offset;
        UniformBuffer* renderable_ub = nullptr;
        UniformBuffer* bone_ub = nullptr;
        std::vector<RenderPrimitive> primitives;
    };

    class RenderPipeline {
    public:
        RenderPipeline();

        ~RenderPipeline();

        void SetScene(Scene* scene);

        void Exec();

        void EnableDebug(bool enable) { _enable_debug = enable; }

    private:
        void AddDebugLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& c);

        void DrawDebugBox(const BBox& bbox, const glm::vec4& c);

        void DrawDebugFrustum(const Frustum& frustum, const glm::vec4& c);

        void ExecDebugStage();

        void ExecUiStage();

        void ComputeCascadeParams(CascadeParameters& cascade_params);

        void UpdateShadowMapInfo(const CascadeParameters& cascade_params, ShadowMapInfo& shadow_map_info);

        void ExecShadowStage();

    private:
        Scene* _scene = nullptr;
        // renderable
        uint32_t _num_ui_renderables;
        std::vector<uint32_t> _ui_renderables;
        uint32_t _num_common_renderables;
        std::vector<uint32_t> _common_renderables;
        uint32_t _num_renderables;
        std::vector<Renderable> _renderables;
        UniformBuffer* _renderable_ub = nullptr;

        // view
        CameraInfo _display_camera_info;
        CameraInfo _main_camera_info;
        uint32_t _num_views;
        std::vector<RenderView> _views;
        UniformBuffer* _view_ub = nullptr;

        // light
        LightInfo _light_info;
        uint32_t _num_lights;
        std::vector<RenderLight> _lights;
        UniformBuffer* _light_ub = nullptr;

        // shadow
        Texture* _shadow_maps[SHADOW_CASCADE_COUNT];
        FramebufferInfo _shadow_map_fb;

        // display
        FramebufferInfo _display_fb;

        // draw call
        uint32_t _dc_head = 0;
        DrawCall _dc_list[10240];

        // debug
        UniformBuffer* _debug_ub = nullptr;
        uint32_t _num_debug_lines;
        std::vector<float> _debug_lines;
        VertexBuffer* _debug_line_vb = nullptr;

        // setting
        bool _enable_debug = false;
    };
}