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
    class Canvas;
    class BaseWindow;
    class Texture;
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

        void RenderWindow(BaseWindow* window, uint32_t view_count, View** views, uint32_t canvas_count, Canvas** canvases);

        Texture* EquirectangularMapToCubemap(Texture* equirectangular_map, uint32_t face_size);

    private:
        void BasePass(Scene* scene, View* view);

        void ShadowPass(Scene* scene, View* view);

        void UpdateShadowMapInfo(Scene* scene, View* view, ShadowMapInfo& shadow_map_info);

        void PostProcessPass(Scene* scene, View* view);

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
         * 三个view uniform buffer
         * 第一个用于场景/ui渲染
         * 第二个用于其他情况
         * 第三个用于窗口渲染
         */
        blast::GfxBuffer* main_view_ub = nullptr;
        blast::GfxBuffer* common_view_ub = nullptr;
        blast::GfxBuffer* window_view_ub = nullptr;
        blast::GfxBuffer* renderable_ub = nullptr;

        ViewUniforms view_storage;
        RenderableUniforms identity_renderable_storage;

        // shadow
        blast::GfxTexture* cascade_shadow_map = nullptr;
        blast::GfxRenderPass* cascade_shadow_passes[SHADOW_CASCADE_COUNT];
        ShadowMapInfo cascade_shadow_map_infos[SHADOW_CASCADE_COUNT];

        DrawCall dc_list[10240];
    };
}