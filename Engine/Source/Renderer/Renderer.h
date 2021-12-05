#pragma once
#include "Core/GearDefine.h"
#include "RenderData.h"

#include <Blast/Gfx/GfxDefine.h>

#include <set>
#include <map>
#include <queue>
#include <functional>

namespace gear {
    class View;
    class Scene;
    class VertexLayoutCache;
    class RasterizerStateCache;
    class DepthStencilStateCache;
    class BlendStateCache;
    class SamplerCache;
    class RenderPassCache;
    class PipelineCache;
    class Renderer {
    public:
        Renderer();

        ~Renderer();

        void RefreshCommandBuffer();

        VertexLayoutCache* GetVertexLayoutCache() { return vertex_layout_cache; }

        RasterizerStateCache* GetRasterizerStateCache() { return rasterizer_state_cache; }

        DepthStencilStateCache* GetDepthStencilStateCache() { return depth_stencil_state_cache; }

        BlendStateCache* GetBlendStateCache() { return blend_state_cache; }

        void RenderScene(Scene* scene, View* view);

    private:
        void BasePass(Scene* scene, View* view);

        void Compose(Scene* scene, View* view);

    private:
        VertexLayoutCache* vertex_layout_cache = nullptr;
        RasterizerStateCache* rasterizer_state_cache = nullptr;
        DepthStencilStateCache* depth_stencil_state_cache = nullptr;
        BlendStateCache* blend_state_cache = nullptr;
        SamplerCache* sampler_cache = nullptr;
        RenderPassCache* renderpass_cache = nullptr;
        PipelineCache* pipeline_cache = nullptr;
        blast::GfxCommandBuffer* current_cmd = nullptr;
        /**
         * 两个个view uniform buffer
         * 第一个用于场景/ui渲染
         * 第二个用于其他情况
         */
        blast::GfxBuffer* main_view_ub = nullptr;
        blast::GfxBuffer* common_view_ub = nullptr;
        blast::GfxBuffer* renderable_ub = nullptr;

        ViewUniforms view_storage;
        RenderableUniforms identity_renderable_storage;

        DrawCall dc_list[10240];
    };
}