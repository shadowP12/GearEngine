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

	void Renderer::RenderTransmittanceLut(gear::Scene* scene, gear::View* view) {
		if (!scene->should_render_atmosphere) {
			return;
		}

		blast::GfxDevice* device = gEngine.GetDevice();

		blast::GfxTextureBarrier barrier[1];
		barrier[0].texture = transmittance_lut;
		barrier[0].new_state = blast::RESOURCE_STATE_RENDERTARGET;
		device->SetBarrier(current_cmd, 0, nullptr, 1, barrier);

		device->RenderPassBegin(current_cmd, transmittance_rp);

		blast::Viewport viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.w = TRANSMITTANCE_TEXTURE_WIDTH;
		viewport.h = TRANSMITTANCE_TEXTURE_HEIGHT;
		device->BindViewports(current_cmd, 1, &viewport);

		blast::Rect rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = TRANSMITTANCE_TEXTURE_WIDTH;
		rect.bottom = TRANSMITTANCE_TEXTURE_HEIGHT;
		device->BindScissorRects(current_cmd, 1, &rect);

		device->BindConstantBuffer(current_cmd, atmosphere_ub, 0, atmosphere_ub->desc.size, 0);

		VertexBuffer* quad_buffer = gEngine.GetBuiltinResources()->GetQuadBuffer();

		blast::GfxShader* vs = gEngine.GetBuiltinResources()->GetAtmosphereCommonVS();
		blast::GfxShader* fs = gEngine.GetBuiltinResources()->GetAtmosphereComputeTransmittanceFS();
		if (vs != nullptr && fs != nullptr) {
			blast::GfxPipelineDesc pipeline_state = {};
			pipeline_state.rp = transmittance_rp;
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

		barrier[0].texture = transmittance_lut;
		barrier[0].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
		device->SetBarrier(current_cmd, 0, nullptr, 1, barrier);
	}

	void Renderer::RenderMultiScattTexture(gear::Scene* scene, gear::View* view) {
		if (!scene->should_render_atmosphere) {
			return;
		}

		blast::GfxDevice* device = gEngine.GetDevice();

		blast::GfxTextureBarrier barrier[1];
		barrier[0].texture = multi_scatt_texture;
		barrier[0].new_state = blast::RESOURCE_STATE_UNORDERED_ACCESS;
		device->SetBarrier(current_cmd, 0, nullptr, 1, barrier);

		blast::GfxShader* cs = gEngine.GetBuiltinResources()->GetAtmosphereComputeMultiScattCS();
		if (cs != nullptr) {
			device->BindConstantBuffer(current_cmd, atmosphere_ub, 0, atmosphere_ub->desc.size, 0);

			blast::GfxSamplerDesc default_sampler = {};
			device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

			device->BindResource(current_cmd, transmittance_lut, 0);

			device->BindUAV(current_cmd, multi_scatt_texture, 0);

			device->BindComputeShader(current_cmd, cs);

			device->Dispatch(current_cmd, std::max(1u, (uint32_t)MULTI_SCATTERING_TEXTURE_SIZE / 16), std::max(1u, (uint32_t)MULTI_SCATTERING_TEXTURE_SIZE / 16), 1);
		}

		barrier[0].texture = multi_scatt_texture;
		barrier[0].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
		device->SetBarrier(current_cmd, 0, nullptr, 1, barrier);
	}

	void Renderer::AtmosphereRayMarching(Scene* scene, View* view) {
		if (!scene->should_render_atmosphere) {
			return;
		}

		view->SwapPostProcess();

		blast::GfxDevice* device = gEngine.GetDevice();

		blast::GfxTextureBarrier barrier[1];
		barrier[0].texture = view->GetOutPostProcessRT();
		barrier[0].new_state = blast::RESOURCE_STATE_RENDERTARGET;
		device->SetBarrier(current_cmd, 0, nullptr, 1, barrier);

		device->RenderPassBegin(current_cmd, view->GetAtmosphereRaymarchingRenderPass());

		blast::Viewport viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.w = view->GetOutPostProcessRT()->desc.width;
		viewport.h = view->GetOutPostProcessRT()->desc.height;
		device->BindViewports(current_cmd, 1, &viewport);

		blast::Rect rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = view->GetOutPostProcessRT()->desc.width;
		rect.bottom = view->GetOutPostProcessRT()->desc.height;
		device->BindScissorRects(current_cmd, 1, &rect);

		VertexBuffer* quad_buffer = gEngine.GetBuiltinResources()->GetQuadBuffer();

		blast::GfxShader* vs = gEngine.GetBuiltinResources()->GetAtmosphereCommonVS();
		blast::GfxShader* fs = gEngine.GetBuiltinResources()->GetAtmosphereRayMarchingFS();
		if (vs != nullptr && fs != nullptr) {
			device->BindConstantBuffer(current_cmd, atmosphere_ub, 0, atmosphere_ub->desc.size, 0);

			blast::GfxSamplerDesc default_sampler = {};
			device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

			device->BindResource(current_cmd, transmittance_lut, 0);

			device->BindResource(current_cmd, multi_scatt_texture, 1);

			device->BindResource(current_cmd, view->depth_rt, 2);

			blast::GfxPipelineDesc pipeline_state = {};
			pipeline_state.rp = view->GetAtmosphereRaymarchingRenderPass();
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