#pragma once
#include "Renderer/RenderData.h"

namespace gear {
    class Renderer;
    class Scene;
    class IndexBuffer;
    class VertexBuffer;
    class UniformBuffer;
    class MaterialInstance;

    struct RenderLight {

    };

    struct RenderView {

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
        uint32_t renderable_ub_offset;
        UniformBuffer* bone_ub = nullptr;
        std::vector<RenderPrimitive> primitives;
    };

    class RenderPipeline {
    public:
        RenderPipeline();

        ~RenderPipeline();

        void SetScene(Scene* scene);

        void Exec();

    private:
        Scene* _scene = nullptr;
        // renderable
        uint32_t _num_renderables;
        std::vector<Renderable> _renderables;
        UniformBuffer* _renderable_ub = nullptr;

        // view
        uint32_t _num_views;
        std::vector<RenderView> _views;
        UniformBuffer* _view_ub = nullptr;

        // light
        uint32_t _num_lights;
        std::vector<RenderLight> _lights;
        UniformBuffer* _light_ub = nullptr;

        // display
        FramebufferInfo display_fb;
    };
}