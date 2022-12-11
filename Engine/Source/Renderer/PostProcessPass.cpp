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

namespace gear {
    void Renderer::PostProcessPass(Scene* scene, View* view) {
        // Copy
        {
            view_storage.view_matrix = glm::mat4(1.0f);
            view_storage.proj_matrix = glm::mat4(1.0f);
            UpdateUniformBuffer(current_cmd, common_view_ub, &view_storage, sizeof(ViewUniforms));
            UpdateUniformBuffer(current_cmd, renderable_ub, &identity_renderable_storage, sizeof(RenderableUniforms));

            blast::GfxResourceBarrier barrier[2];
            barrier[0].resource = view->main_rt;
            barrier[0].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
            barrier[1].resource = view->GetOutPostProcessRT();
            barrier[1].new_state = blast::RESOURCE_STATE_RENDERTARGET;
            device->SetBarrier(current_cmd, 2, barrier);

            blast::GfxRenderPassDesc renderpass_desc = {};
            renderpass_desc.attachments.push_back(blast::RenderPassAttachment::RenderTarget(view->GetOutPostProcessRT(), -1, blast::LOAD_CLEAR));
            blast::GfxRenderPass* renderpass = renderpass_cache->GetRenderPass(renderpass_desc);
            device->RenderPassBegin(current_cmd, renderpass);

            device->BindConstantBuffer(current_cmd, common_view_ub, 1, common_view_ub->size, 0);
            device->BindConstantBuffer(current_cmd, renderable_ub, 2, renderable_ub->size, 0);

            device->BindResource(current_cmd, view->main_rt, 0);

            blast::GfxSamplerDesc default_sampler = {};
            device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

            blast::GfxBuffer* quad_buffer = gEngine.GetBuiltinResources()->GetQuadBuffer().get();
            blast::GfxShader* vs = gEngine.GetBuiltinResources()->GetBlitMaterial()->GetVertShader(0, VertexLayoutType::VLT_P_T0);
            blast::GfxShader* fs = gEngine.GetBuiltinResources()->GetBlitMaterial()->GetFragShader(0, VertexLayoutType::VLT_P_T0);
            if (vs != nullptr && fs != nullptr) {
                blast::GfxPipelineDesc pipeline_state = {};
                pipeline_state.rp = renderpass;
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

            device->RenderPassEnd(current_cmd);

            barrier[0].resource = view->GetOutPostProcessRT();
            barrier[0].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
            device->SetBarrier(current_cmd, 1, barrier);
        }

        // Atmosphere ray marching
        AtmosphereRayMarching(scene, view);

        // FXAA
        {
            view->SwapPostProcess();

            view_storage.view_matrix = glm::mat4(1.0f);
            view_storage.proj_matrix = glm::mat4(1.0f);
            device->UpdateBuffer(current_cmd, common_view_ub, &view_storage, sizeof(ViewUniforms));
            device->UpdateBuffer(current_cmd, renderable_ub, &identity_renderable_storage, sizeof(RenderableUniforms));

            blast::GfxResourceBarrier barrier[1];
            barrier[0].resource = view->GetOutPostProcessRT();
            barrier[0].new_state = blast::RESOURCE_STATE_RENDERTARGET;
            device->SetBarrier(current_cmd, 1, barrier);

            blast::GfxRenderPassDesc renderpass_desc = {};
            renderpass_desc.attachments.push_back(blast::RenderPassAttachment::RenderTarget(view->GetOutPostProcessRT(), -1, blast::LOAD_CLEAR));
            blast::GfxRenderPass* renderpass = renderpass_cache->GetRenderPass(renderpass_desc);
            device->RenderPassBegin(current_cmd, renderpass);

            device->BindConstantBuffer(current_cmd, common_view_ub, 1, common_view_ub->size, 0);
            device->BindConstantBuffer(current_cmd, renderable_ub, 2, renderable_ub->size, 0);

            device->BindResource(current_cmd, view->GetInPostProcessRT(), 0);

            blast::GfxSamplerDesc default_sampler = {};
            device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

            blast::GfxBuffer* quad_buffer = gEngine.GetBuiltinResources()->GetQuadBuffer().get();
            blast::GfxShader* vs = gEngine.GetBuiltinResources()->GetFXAAMaterial()->GetVertShader(0, VertexLayoutType::VLT_P_T0);
            blast::GfxShader* fs = gEngine.GetBuiltinResources()->GetFXAAMaterial()->GetFragShader(0, VertexLayoutType::VLT_P_T0);
            if (vs != nullptr && fs != nullptr) {
                blast::GfxPipelineDesc pipeline_state = {};
                pipeline_state.rp = renderpass;
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

            device->RenderPassEnd(current_cmd);

            barrier[0].resource = view->GetOutPostProcessRT();
            barrier[0].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
            device->SetBarrier(current_cmd, 1, barrier);
        }
    }
}