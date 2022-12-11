#include "Renderer.h"
#include "GearEngine.h"
#include "RenderCache.h"
#include "View/View.h"
#include "Window/BaseWindow.h"
#include "Entity/Scene.h"
#include "UI/Canvas.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Resource/BuiltinResources.h"

#include <GfxDevice.h>

namespace gear {
    void Renderer::DebugPass(Scene* scene, View* view) {
        if (view->num_debug_lines == 0) {
            return;
        }

        view->SwapPostProcess();

        UpdateUniformBuffer(current_cmd, debug_line_vb, view->debug_lines.data(), view->num_debug_lines * sizeof(float) * 14);

        ViewUniforms vb_storage = view_storage;
        UpdateUniformBuffer(current_cmd, common_view_ub, &vb_storage, sizeof(ViewUniforms));
        UpdateUniformBuffer(current_cmd, renderable_ub, &identity_renderable_storage, sizeof(RenderableUniforms));

        blast::GfxResourceBarrier barrier[1];
        barrier[0].resource = view->GetOutPostProcessRT();
        barrier[0].new_state = blast::RESOURCE_STATE_RENDERTARGET;
        device->SetBarrier(current_cmd, 1, barrier);

        blast::GfxPipelineDesc pipeline_state = {};
        blast::GfxRenderPassDesc renderpass_desc = {};
        renderpass_desc.attachments.push_back(blast::RenderPassAttachment::RenderTarget(view->GetOutPostProcessRT(), -1, blast::LOAD_CLEAR));
        blast::GfxRenderPass* renderpass = renderpass_cache->GetRenderPass(renderpass_desc);
        pipeline_state.rp = renderpass;
        device->RenderPassBegin(current_cmd, renderpass);

        device->BindViewport(current_cmd, 0, 0, view->main_rt->width, view->main_rt->height);

        device->BindScissor(current_cmd, 0, 0, view->main_rt->width, view->main_rt->height);

        {
            // Blit
            device->BindConstantBuffer(current_cmd, common_view_ub, 1, common_view_ub->size, 0);
            device->BindConstantBuffer(current_cmd, renderable_ub, 2, renderable_ub->size, 0);
            device->BindResource(current_cmd, view->GetInPostProcessRT(), 0);

            blast::GfxSamplerDesc default_sampler = {};
            device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

            blast::GfxBuffer* quad_buffer = gEngine.GetBuiltinResources()->GetQuadBuffer().get();
            blast::GfxShader* vs = gEngine.GetBuiltinResources()->GetBlitMaterial()->GetVertShader(0, VLT_P_T0);
            blast::GfxShader* fs = gEngine.GetBuiltinResources()->GetBlitMaterial()->GetFragShader(0, VLT_P_T0);
            if (vs != nullptr && fs != nullptr) {
                pipeline_state.vs = vs;
                pipeline_state.fs = fs;
                pipeline_state.il = vertex_layout_cache->GetVertexLayout(VertexLayoutType::VLT_P_T0);
                pipeline_state.rs = rasterizer_state_cache->GetRasterizerState(RST_DOUBLESIDED);
                pipeline_state.bs = blend_state_cache->GetDepthStencilState(BST_OPAQUE);
                pipeline_state.dss = depth_stencil_state_cache->GetDepthStencilState(DSST_UI);

                device->BindPipeline(current_cmd, pipeline_cache->GetPipeline(pipeline_state));

                uint64_t vertex_offsets[] = {0};
                blast::GfxBuffer* vertex_buffers[] = {quad_buffer};
                device->BindVertexBuffers(current_cmd, vertex_buffers, 0, 1, vertex_offsets);

                device->Draw(current_cmd, 6, 0);
            }
        }

        {
            // Draw debug line
            device->BindConstantBuffer(current_cmd, main_view_ub, 1, main_view_ub->size, 0);
            device->BindConstantBuffer(current_cmd, renderable_ub, 2, renderable_ub->size, 0);
            blast::GfxShader* vs = gEngine.GetBuiltinResources()->GetDebugMaterial()->GetVertShader(0, VLT_DEBUG);
            blast::GfxShader* fs = gEngine.GetBuiltinResources()->GetDebugMaterial()->GetFragShader(0, VLT_DEBUG);
            if (vs != nullptr && fs != nullptr) {
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
        }

        device->RenderPassEnd(current_cmd);

        barrier[0].resource = view->GetOutPostProcessRT();
        barrier[0].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(current_cmd, 1, barrier);

        view->num_debug_lines = 0;
    }
}