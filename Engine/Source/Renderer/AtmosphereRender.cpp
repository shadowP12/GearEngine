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

    void Renderer::RenderTransmittanceLut(gear::Scene* scene, gear::View* view) {
        if (!scene->should_render_atmosphere) {
            return;
        }

        blast::GfxResourceBarrier barrier[1];
        barrier[0].resource = transmittance_lut;
        barrier[0].new_state = blast::RESOURCE_STATE_RENDERTARGET;
        device->SetBarrier(current_cmd, 1, barrier);

        device->RenderPassBegin(current_cmd, transmittance_rp);

        device->BindViewport(current_cmd, 0.0f, 0.0f, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);

        device->BindScissor(current_cmd, 0, 0, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);

        device->BindConstantBuffer(current_cmd, atmosphere_ub, 0, atmosphere_ub->size, 0);

        blast::GfxBuffer* quad_buffer = gEngine.GetBuiltinResources()->GetQuadBuffer().get();
        blast::GfxShader* vs = gEngine.GetBuiltinResources()->GetAtmosphereCommonVS().get();
        blast::GfxShader* fs = gEngine.GetBuiltinResources()->GetAtmosphereComputeTransmittanceFS().get();
        if (vs != nullptr && fs != nullptr) {
            blast::GfxPipelineDesc pipeline_state = {};
            pipeline_state.rp = transmittance_rp;
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

        barrier[0].resource = transmittance_lut;
        barrier[0].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(current_cmd, 1, barrier);
    }

    void Renderer::RenderMultiScattTexture(gear::Scene* scene, gear::View* view) {
        if (!scene->should_render_atmosphere) {
            return;
        }

        blast::GfxResourceBarrier barrier[1];
        barrier[0].resource = multi_scatt_texture;
        barrier[0].new_state = blast::RESOURCE_STATE_UNORDERED_ACCESS;
        device->SetBarrier(current_cmd, 1, barrier);

        blast::GfxShader* cs = gEngine.GetBuiltinResources()->GetAtmosphereComputeMultiScattCS().get();
        if (cs != nullptr) {
            device->BindConstantBuffer(current_cmd, atmosphere_ub, 0, atmosphere_ub->size, 0);

            blast::GfxSamplerDesc default_sampler = {};
            device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

            device->BindResource(current_cmd, transmittance_lut, 0);

            device->BindUAV(current_cmd, multi_scatt_texture, 0);

            device->BindComputeShader(current_cmd, cs);

            device->Dispatch(current_cmd, std::max(1u, (uint32_t)MULTI_SCATTERING_TEXTURE_SIZE / 16), std::max(1u, (uint32_t)MULTI_SCATTERING_TEXTURE_SIZE / 16), 1);
        }

        barrier[0].resource = multi_scatt_texture;
        barrier[0].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(current_cmd, 1, barrier);
    }

    void Renderer::AtmosphereRayMarching(Scene* scene, View* view) {
        if (!scene->should_render_atmosphere) {
            return;
        }

        view->SwapPostProcess();

        blast::GfxResourceBarrier barrier[4];
        barrier[0].resource = view->GetOutPostProcessRT();
        barrier[0].new_state = blast::RESOURCE_STATE_RENDERTARGET;
        barrier[1].resource = view->depth_rt;
        barrier[1].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        barrier[2].resource = transmittance_lut;
        barrier[2].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        barrier[3].resource = multi_scatt_texture;
        barrier[3].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(current_cmd, 4, barrier);

        device->RenderPassBegin(current_cmd, view->GetAtmosphereRaymarchingRenderPass());

        device->BindViewport(current_cmd, 0.0f, 0.0f, view->GetOutPostProcessRT()->width, view->GetOutPostProcessRT()->height);

        device->BindScissor(current_cmd, 0, 0, view->GetOutPostProcessRT()->width, view->GetOutPostProcessRT()->height);

        blast::GfxBuffer* quad_buffer = gEngine.GetBuiltinResources()->GetQuadBuffer().get();
        blast::GfxShader* vs = gEngine.GetBuiltinResources()->GetAtmosphereCommonVS().get();
        blast::GfxShader* fs = gEngine.GetBuiltinResources()->GetAtmosphereRayMarchingFS().get();
        if (vs != nullptr && fs != nullptr) {
            device->BindConstantBuffer(current_cmd, atmosphere_ub, 0, atmosphere_ub->size, 0);

            blast::GfxSamplerDesc default_sampler = {};
            device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

            device->BindResource(current_cmd, transmittance_lut, 0);

            device->BindResource(current_cmd, multi_scatt_texture, 1);

            device->BindResource(current_cmd, view->depth_rt, 2);

            blast::GfxPipelineDesc pipeline_state = {};
            pipeline_state.rp = view->GetAtmosphereRaymarchingRenderPass();
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