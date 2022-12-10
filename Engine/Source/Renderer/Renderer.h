#pragma once
#include "Core/GearDefine.h"
#include "RenderData.h"
#include <GfxDefine.h>
#include <GfxDevice.h>
#include <GfxShaderCompiler.h>
#include <set>
#include <map>
#include <queue>
#include <functional>

namespace gear {
    class View;
    class Scene;
    class Canvas;
    class BaseWindow;
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

        void Tick(float dt);

        VertexLayoutCache* GetVertexLayoutCache() { return vertex_layout_cache; }

        RasterizerStateCache* GetRasterizerStateCache() { return rasterizer_state_cache; }

        DepthStencilStateCache* GetDepthStencilStateCache() { return depth_stencil_state_cache; }

        BlendStateCache* GetBlendStateCache() { return blend_state_cache; }

        void RenderScene(Scene* scene, View* view);

        void RenderWindow(BaseWindow* window, uint32_t view_count, View** views, uint32_t canvas_count, Canvas** canvases);

        std::shared_ptr<blast::GfxTexture> EquirectangularMapToCubemap(std::shared_ptr<blast::GfxTexture> equirectangular_map, uint32_t face_size);

        std::shared_ptr<blast::GfxTexture> ComputeIrradianceMap(std::shared_ptr<blast::GfxTexture> cube_map);

        std::shared_ptr<blast::GfxTexture> ComputePrefilteredMap(std::shared_ptr<blast::GfxTexture> cube_map);

        std::shared_ptr<blast::GfxTexture> ComputeBRDFLut();

        blast::GfxDevice* GetDevice() { return device; };

        blast::GfxShaderCompiler* GetShaderCompiler() { return shader_compiler; }

        blast::GfxCommandBuffer* GetCurrentCommandBuffer() { return current_cmd; }

        void UpdateUniformBuffer(blast::GfxCommandBuffer* cmd, blast::GfxBuffer* buffer, const void* data, uint64_t size = 0, uint64_t offset = 0);

        void UpdateVertexBuffer(blast::GfxCommandBuffer* cmd, blast::GfxBuffer* buffer, const void* data, uint64_t size = 0, uint64_t offset = 0);

        void UpdateIndexBuffer(blast::GfxCommandBuffer* cmd, blast::GfxBuffer* buffer, const void* data, uint64_t size = 0, uint64_t offset = 0);

    private:
        void BasePass(Scene* scene, View* view);

        void ShadowPass(Scene* scene, View* view);

        void UpdateShadowMapInfo(Scene* scene, View* view, BBox ws_shadow_receivers_volume, ShadowMapInfo& shadow_map_info);

        void PostProcessPass(Scene* scene, View* view);

        void DebugPass(Scene* scene, View* view);

        void RenderTransmittanceLut(Scene* scene, View* view);

        void RenderMultiScattTexture(Scene* scene, View* view);

        void AtmosphereRayMarching(Scene* scene, View* view);

    private:
        blast::GfxDevice* device = nullptr;
        blast::GfxShaderCompiler* shader_compiler = nullptr;
        blast::GfxCommandBuffer* current_cmd = nullptr;
        blast::GfxBuffer* main_view_ub = nullptr;
        blast::GfxBuffer* common_view_ub = nullptr;
        blast::GfxBuffer* window_view_ub = nullptr;
        blast::GfxBuffer* renderable_ub = nullptr;
        blast::GfxBuffer* atmosphere_ub = nullptr;

        ViewUniforms view_storage;
        RenderableUniforms identity_renderable_storage;

        // Shadow
        blast::GfxTexture* cascade_shadow_map = nullptr;
        blast::GfxRenderPass* cascade_shadow_passes[SHADOW_CASCADE_COUNT];
        ShadowMapInfo cascade_shadow_map_infos[SHADOW_CASCADE_COUNT];

        // Atmosphere
        blast::GfxTexture* transmittance_lut = nullptr;
        blast::GfxRenderPass* transmittance_rp = nullptr;
        blast::GfxTexture* multi_scatt_texture = nullptr;

        // Debug
        blast::GfxBuffer* debug_line_vb = nullptr;

        DrawCall dc_list[10240];

        VertexLayoutCache* vertex_layout_cache = nullptr;
        RasterizerStateCache* rasterizer_state_cache = nullptr;
        DepthStencilStateCache* depth_stencil_state_cache = nullptr;
        BlendStateCache* blend_state_cache = nullptr;
        SamplerCache* sampler_cache = nullptr;
        RenderPassCache* renderpass_cache = nullptr;
        PipelineCache* pipeline_cache = nullptr;
    };
}