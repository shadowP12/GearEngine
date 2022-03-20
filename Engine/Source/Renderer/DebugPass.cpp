#include "Renderer.h"
#include "GearEngine.h"
#include "RenderCache.h"
#include "View/View.h"
#include "Window/BaseWindow.h"
#include "Entity/Scene.h"
#include "UI/Canvas.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Resource/BuiltinResources.h"

#include <Blast/Gfx/GfxDevice.h>

namespace gear {
    void Renderer::DebugPass(Scene* scene, View* view) {
        view->num_debug_lines = 0;
        return;

        if (view->num_debug_lines == 0) {
            return;
        }

        blast::GfxDevice* device = gEngine.GetDevice();
        device->UpdateBuffer(current_cmd, debug_line_vb, view->debug_lines.data(), view->num_debug_lines * sizeof(float) * 14);

        ViewUniforms vb_storage = view_storage;
        device->UpdateBuffer(current_cmd, common_view_ub, &vb_storage, sizeof(ViewUniforms));
        device->UpdateBuffer(current_cmd, renderable_ub, &identity_renderable_storage, sizeof(RenderableUniforms));

        view->SwapPostProcess();

        blast::GfxTextureBarrier barrier[1];
        barrier[0].texture = view->GetOutPostProcessRT();
        barrier[0].new_state = blast::RESOURCE_STATE_RENDERTARGET;
        device->SetBarrier(current_cmd, 0, nullptr, 1, barrier);

        blast::GfxPipelineDesc pipeline_state = {};
        pipeline_state.rp = view->renderpass;
        device->RenderPassBegin(current_cmd, view->renderpass);

        blast::Viewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.w = view->main_rt->desc.width;
        viewport.h = view->main_rt->desc.height;
        device->BindViewports(current_cmd, 1, &viewport);

        blast::Rect rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = view->main_rt->desc.width;
        rect.bottom = view->main_rt->desc.height;
        device->BindScissorRects(current_cmd, 1, &rect);

        device->BindConstantBuffer(current_cmd, common_view_ub, 1, common_view_ub->desc.size, 0);
        device->BindConstantBuffer(current_cmd, renderable_ub, 2, renderable_ub->desc.size, 0);

        blast::GfxShader* vs = gEngine.GetBuiltinResources()->GetDebugMaterial()->GetVertShader(0, VLT_DEBUG);
        blast::GfxShader* fs = gEngine.GetBuiltinResources()->GetDebugMaterial()->GetFragShader(0, VLT_DEBUG);
        if (vs != nullptr && fs != nullptr) {
            pipeline_state.rp = view->renderpass;
            pipeline_state.vs = vs;
            pipeline_state.fs = fs;
            pipeline_state.il = vertex_layout_cache->GetVertexLayout(VLT_DEBUG);
            pipeline_state.rs = rasterizer_state_cache->GetRasterizerState(RST_DOUBLESIDED);
            pipeline_state.bs = blend_state_cache->GetDepthStencilState(BST_OPAQUE);
            pipeline_state.dss = depth_stencil_state_cache->GetDepthStencilState(DSST_UI);
            pipeline_state.primitive_topo = blast::PRIMITIVE_TOPO_LINE_LIST;

            device->BindPipeline(current_cmd, pipeline_cache->GetPipeline(pipeline_state));

            uint64_t vertex_offsets[] = {0};
            blast::GfxBuffer* vertex_buffers[] = {debug_line_vb};
            device->BindVertexBuffers(current_cmd, vertex_buffers, 0, 1, vertex_offsets);

            device->Draw(current_cmd, view->num_debug_lines * 2, 0);
        }

        device->RenderPassEnd(current_cmd);

        barrier[0].texture = view->GetOutPostProcessRT();
        barrier[0].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(current_cmd, 0, nullptr, 1, barrier);

        view->num_debug_lines = 0;
    }
}