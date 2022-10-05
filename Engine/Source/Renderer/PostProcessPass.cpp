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
    void Renderer::PostProcessPass(Scene* scene, View* view) {
        blast::GfxDevice* device = gEngine.GetDevice();

		// copy
		{
			view_storage.view_matrix = glm::mat4(1.0f);
			view_storage.proj_matrix = glm::mat4(1.0f);
			device->UpdateBuffer(current_cmd, common_view_ub, &view_storage, sizeof(ViewUniforms));
			device->UpdateBuffer(current_cmd, renderable_ub, &identity_renderable_storage, sizeof(RenderableUniforms));

			blast::GfxTextureBarrier barrier[1];
			barrier[0].texture = view->GetOutPostProcessRT();
			barrier[0].new_state = blast::RESOURCE_STATE_RENDERTARGET;
			device->SetBarrier(current_cmd, 0, nullptr, 1, barrier);

			device->RenderPassBegin(current_cmd, view->GetFXAARenderPass());

			device->BindConstantBuffer(current_cmd, common_view_ub, 1, common_view_ub->desc.size, 0);
			device->BindConstantBuffer(current_cmd, renderable_ub, 2, renderable_ub->desc.size, 0);

			device->BindResource(current_cmd, view->main_rt, 0);

			blast::GfxSamplerDesc default_sampler = {};
			device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

			VertexBuffer* quad_buffer = gEngine.GetBuiltinResources()->GetQuadBuffer();

			blast::GfxShader* vs = gEngine.GetBuiltinResources()->GetBlitMaterial()->GetVertShader(0, quad_buffer->GetVertexLayoutType());
			blast::GfxShader* fs = gEngine.GetBuiltinResources()->GetBlitMaterial()->GetFragShader(0, quad_buffer->GetVertexLayoutType());
			if (vs != nullptr && fs != nullptr) {
				blast::GfxPipelineDesc pipeline_state = {};
				pipeline_state.rp = view->GetFXAARenderPass();
				pipeline_state.vs = vs;
				pipeline_state.fs = fs;
				pipeline_state.il = vertex_layout_cache->GetVertexLayout(quad_buffer->GetVertexLayoutType());
				pipeline_state.rs = rasterizer_state_cache->GetRasterizerState(RST_DOUBLESIDED);
				pipeline_state.bs = blend_state_cache->GetDepthStencilState(BST_OPAQUE);
				pipeline_state.dss = depth_stencil_state_cache->GetDepthStencilState(DSST_UI);

				device->BindPipeline(current_cmd, pipeline_cache->GetPipeline(pipeline_state));

				uint64_t vertex_offsets[] = {0};
				blast::GfxBuffer* vertex_buffers[] = {quad_buffer->GetHandle()};
				device->BindVertexBuffers(current_cmd, vertex_buffers, 0, 1, vertex_offsets);

				device->Draw(current_cmd, 6, 0);
			}

			device->RenderPassEnd(current_cmd);

			barrier[0].texture = view->GetOutPostProcessRT();
			barrier[0].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
			device->SetBarrier(current_cmd, 0, nullptr, 1, barrier);
		}

        // fxaa
        {
			view->SwapPostProcess();

			view_storage.view_matrix = glm::mat4(1.0f);
			view_storage.proj_matrix = glm::mat4(1.0f);
			device->UpdateBuffer(current_cmd, common_view_ub, &view_storage, sizeof(ViewUniforms));
			device->UpdateBuffer(current_cmd, renderable_ub, &identity_renderable_storage, sizeof(RenderableUniforms));

            blast::GfxTextureBarrier barrier[1];
            barrier[0].texture = view->GetOutPostProcessRT();
            barrier[0].new_state = blast::RESOURCE_STATE_RENDERTARGET;
            device->SetBarrier(current_cmd, 0, nullptr, 1, barrier);

            device->RenderPassBegin(current_cmd, view->GetFXAARenderPass());

            device->BindConstantBuffer(current_cmd, common_view_ub, 1, common_view_ub->desc.size, 0);
            device->BindConstantBuffer(current_cmd, renderable_ub, 2, renderable_ub->desc.size, 0);

            device->BindResource(current_cmd, view->GetInPostProcessRT(), 0);

            blast::GfxSamplerDesc default_sampler = {};
            device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

            VertexBuffer* quad_buffer = gEngine.GetBuiltinResources()->GetQuadBuffer();

            blast::GfxShader* vs = gEngine.GetBuiltinResources()->GetFXAAMaterial()->GetVertShader(0, quad_buffer->GetVertexLayoutType());
            blast::GfxShader* fs = gEngine.GetBuiltinResources()->GetFXAAMaterial()->GetFragShader(0, quad_buffer->GetVertexLayoutType());
            if (vs != nullptr && fs != nullptr) {
                blast::GfxPipelineDesc pipeline_state = {};
                pipeline_state.rp = view->GetFXAARenderPass();
                pipeline_state.vs = vs;
                pipeline_state.fs = fs;
                pipeline_state.il = vertex_layout_cache->GetVertexLayout(quad_buffer->GetVertexLayoutType());
                pipeline_state.rs = rasterizer_state_cache->GetRasterizerState(RST_DOUBLESIDED);
                pipeline_state.bs = blend_state_cache->GetDepthStencilState(BST_OPAQUE);
                pipeline_state.dss = depth_stencil_state_cache->GetDepthStencilState(DSST_UI);

                device->BindPipeline(current_cmd, pipeline_cache->GetPipeline(pipeline_state));

                uint64_t vertex_offsets[] = {0};
                blast::GfxBuffer* vertex_buffers[] = {quad_buffer->GetHandle()};
                device->BindVertexBuffers(current_cmd, vertex_buffers, 0, 1, vertex_offsets);

                device->Draw(current_cmd, 6, 0);
            }

            device->RenderPassEnd(current_cmd);

            barrier[0].texture = view->GetOutPostProcessRT();
            barrier[0].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
            device->SetBarrier(current_cmd, 0, nullptr, 1, barrier);
        }


    }
}