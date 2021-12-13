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
    Renderer::Renderer() {
        RefreshCommandBuffer();

        vertex_layout_cache = new VertexLayoutCache();
        rasterizer_state_cache = new RasterizerStateCache();
        depth_stencil_state_cache = new DepthStencilStateCache();
        blend_state_cache = new BlendStateCache();
        sampler_cache = new SamplerCache();
        renderpass_cache = new RenderPassCache();
        pipeline_cache = new PipelineCache();

        // 初始化渲染器资源
        blast::GfxBufferDesc buffer_desc = {};
        buffer_desc.size = sizeof(ViewUniforms);
        buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
        buffer_desc.res_usage = blast::RESOURCE_USAGE_UNIFORM_BUFFER;
        main_view_ub = gEngine.GetDevice()->CreateBuffer(buffer_desc);
        common_view_ub = gEngine.GetDevice()->CreateBuffer(buffer_desc);
        window_view_ub = gEngine.GetDevice()->CreateBuffer(buffer_desc);

        buffer_desc.size = sizeof(RenderableUniforms);
        buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
        buffer_desc.res_usage = blast::RESOURCE_USAGE_UNIFORM_BUFFER;
        renderable_ub = gEngine.GetDevice()->CreateBuffer(buffer_desc);

        view_storage.view_matrix = glm::mat4(1.0f);
        view_storage.proj_matrix = glm::mat4(1.0f);
        identity_renderable_storage.model_matrix = glm::mat4(1.0);
        identity_renderable_storage.normal_matrix = glm::mat4(1.0);
    }

    Renderer::~Renderer() {
        blast::GfxDevice* device = gEngine.GetDevice();
        device->DestroyBuffer(main_view_ub);
        device->DestroyBuffer(common_view_ub);
        device->DestroyBuffer(window_view_ub);
        device->DestroyBuffer(renderable_ub);

        SAFE_DELETE(vertex_layout_cache);
        SAFE_DELETE(rasterizer_state_cache);
        SAFE_DELETE(depth_stencil_state_cache);
        SAFE_DELETE(blend_state_cache);
        SAFE_DELETE(sampler_cache);
        SAFE_DELETE(renderpass_cache);
        SAFE_DELETE(pipeline_cache);
        SAFE_DELETE(vertex_layout_cache);
    }

    void Renderer::RefreshCommandBuffer() {
        current_cmd = gEngine.GetDevice()->RequestCommandBuffer(blast::QUEUE_GRAPHICS);
    }

    void Renderer::RenderScene(Scene* scene, View* view) {
        if (!scene->Prepare(current_cmd)) {
            return;
        }

        if (!view->Prepare(current_cmd)) {
            return;
        }

        blast::GfxDevice* device = gEngine.GetDevice();
        view_storage.view_matrix = scene->display_camera_info.view;
        view_storage.proj_matrix = scene->display_camera_info.projection;
        view_storage.main_view_matrix = scene->main_camera_info.view;
        device->UpdateBuffer(current_cmd, main_view_ub, &view_storage, sizeof(ViewUniforms));

        BasePass(scene, view);
    }

    void Renderer::BasePass(Scene* scene, View* view) {
        uint32_t dc_head = 0;
        uint32_t dc_count = 0;
        for (uint32_t i = 0; i < scene->num_mesh_renderables; ++i) {
            Renderable* rb = &scene->renderables[scene->mesh_renderables[i]];
            for (uint32_t j = 0; j < rb->num_primitives; ++j) {
                RenderPrimitive* rp = &rb->primitives[j];
                uint32_t dc_idx = dc_head + dc_count;
                uint32_t material_id = rp->mi->GetMaterial()->GetMaterialID();
                uint32_t material_instance_id = rp->mi->GetMaterialInstanceID();
                uint32_t material_variant = 0;
                if (scene->light_info.has_direction_light) {
                    material_variant |= MaterialVariant::DIRECTIONAL_LIGHTING;
                }

                if (rp->receive_shadow) {
                    // TODO: 暂时屏蔽阴影
                    // material_variant |= MaterialVariant::SHADOW_RECEIVER;
                }

                dc_list[dc_idx] = {};
                dc_list[dc_idx].renderable_id = scene->mesh_renderables[i];
                dc_list[dc_idx].primitive_id = j;
                dc_list[dc_idx].material_variant = material_variant;
                dc_list[dc_idx].key |= DrawCall::GenMaterialKey(material_id, material_variant, material_instance_id);

                dc_count++;
            }
        }
        std::sort(&dc_list[dc_head], &dc_list[dc_head] + dc_count);

        blast::GfxDevice* device = gEngine.GetDevice();

        // image layout to rp
        {
            blast::GfxTextureBarrier barrier[2];
            barrier[0].texture = view->main_rt;
            barrier[0].new_state = blast::RESOURCE_STATE_RENDERTARGET;
            barrier[1].texture = view->depth_rt;
            barrier[1].new_state = blast::RESOURCE_STATE_DEPTH_WRITE;
            device->SetBarrier(current_cmd, 0, nullptr, 2, barrier);
        }

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

        for (uint32_t i = dc_head; i < dc_head + dc_count; ++i) {
            uint32_t material_variant = dc_list[i].material_variant;
            uint32_t randerable_id = dc_list[i].renderable_id;
            uint32_t primitive_id = dc_list[i].primitive_id;
            Renderable& renderable = scene->renderables[randerable_id];
            RenderPrimitive& primitive = renderable.primitives[primitive_id];

            device->BindConstantBuffer(current_cmd, main_view_ub, 1, main_view_ub->desc.size, 0);
            device->BindConstantBuffer(current_cmd, renderable.renderable_ub, 2, renderable.renderable_ub_size, renderable.renderable_ub_offset);

            // 材质参数
            if (primitive.material_ub != nullptr) {
                device->BindConstantBuffer(current_cmd, primitive.material_ub->GetHandle(), 0, primitive.material_ub->GetSize(), 0);
            }

            for (auto& sampler_item : primitive.mi->GetGfxSamplerGroup()) {
                device->BindSampler(current_cmd, sampler_cache->GetSampler(sampler_item.second), sampler_item.first);
            }

            for (auto& texture_item : primitive.mi->GetGfxTextureGroup()) {
                device->BindResource(current_cmd, texture_item.second->GetTexture(), texture_item.first);
            }

            pipeline_state.vs = primitive.mi->GetMaterial()->GetVertShader(material_variant, primitive.vb->GetVertexLayoutType());
            pipeline_state.fs = primitive.mi->GetMaterial()->GetFragShader(material_variant, primitive.vb->GetVertexLayoutType());
            pipeline_state.il = vertex_layout_cache->GetVertexLayout(primitive.vb->GetVertexLayoutType());
            pipeline_state.rs = rasterizer_state_cache->GetRasterizerState(RST_DOUBLESIDED);
            pipeline_state.bs = blend_state_cache->GetDepthStencilState(primitive.mi->GetMaterial()->GetBlendState());
            pipeline_state.dss = depth_stencil_state_cache->GetDepthStencilState(DSST_UI);

            device->BindPipeline(current_cmd, pipeline_cache->GetPipeline(pipeline_state));

            uint64_t vertex_offsets[] = {0};
            blast::GfxBuffer* vertex_buffers[] = {primitive.vb->GetHandle()};
            device->BindVertexBuffers(current_cmd, vertex_buffers, 0, 1, vertex_offsets);

            device->BindIndexBuffer(current_cmd, primitive.ib->GetHandle(), primitive.ib->GetIndexType(), 0);

            device->DrawIndexed(current_cmd, primitive.count, primitive.offset, 0);
        }
        device->RenderPassEnd(current_cmd);

        // image layout to shader
        {
            blast::GfxTextureBarrier barrier[2];
            barrier[0].texture = view->main_rt;
            barrier[0].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
            barrier[1].texture = view->depth_rt;
            barrier[1].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
            device->SetBarrier(current_cmd, 0, nullptr, 2, barrier);
        }
    }

    void Renderer::RenderWindow(BaseWindow* window, uint32_t view_count, View** views, uint32_t canvas_count, Canvas** canvases) {
        if (view_count == 0 && canvas_count == 0) {
            return;
        }

        blast::GfxSwapChain* swapchain = window->GetSwapChain();
        if (!swapchain) {
            return;
        }

        std::vector<View*> valid_views;
        for (uint32_t i = 0; i < view_count; ++i) {
            if (views[i]->Prepare(current_cmd)) {
                valid_views.push_back(views[i]);
            }
        }

        std::vector<Canvas*> valid_canvases;
        for (uint32_t i = 0; i < canvas_count; ++i) {
            if (canvases[i]->Prepare(current_cmd)) {
                valid_canvases.push_back(canvases[i]);
            }
        }

        blast::GfxDevice* device = gEngine.GetDevice();

        blast::Viewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.w = window->GetWidth();
        viewport.h = window->GetHeight();
        device->BindViewports(current_cmd, 1, &viewport);

        blast::Rect rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = window->GetWidth();
        rect.bottom = window->GetHeight();

        view_storage.view_matrix = glm::mat4(1.0f);
        view_storage.proj_matrix = glm::mat4(1.0f);
        device->UpdateBuffer(current_cmd, common_view_ub, &view_storage, sizeof(ViewUniforms));
        device->UpdateBuffer(current_cmd, renderable_ub, &identity_renderable_storage, sizeof(RenderableUniforms));

        view_storage.view_matrix = glm::mat4(1.0f);
        view_storage.proj_matrix = glm::ortho(0.0f, (float)window->GetWidth(), 0.0f, (float)window->GetHeight(), 0.0f, 10.0f);
        device->UpdateBuffer(current_cmd, window_view_ub, &view_storage, sizeof(ViewUniforms));

        device->RenderPassBegin(current_cmd, swapchain);

        for (uint32_t i = 0; i < valid_views.size(); ++i) {
            device->BindConstantBuffer(current_cmd, common_view_ub, 1, common_view_ub->desc.size, 0);
            device->BindConstantBuffer(current_cmd, renderable_ub, 2, renderable_ub->desc.size, 0);

            device->BindResource(current_cmd, valid_views[i]->main_rt, 0);

            blast::GfxSamplerDesc default_sampler = {};
            device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

            VertexBuffer* quad_buffer = gEngine.GetBuiltinResources()->GetQuadBuffer();

            blast::GfxPipelineDesc pipeline_state = {};
            pipeline_state.sc = swapchain;;
            pipeline_state.vs = gEngine.GetBuiltinResources()->GetBlitMaterial()->GetVertShader(0, quad_buffer->GetVertexLayoutType());
            pipeline_state.fs = gEngine.GetBuiltinResources()->GetBlitMaterial()->GetFragShader(0, quad_buffer->GetVertexLayoutType());
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

        for (uint32_t i = 0; i < valid_canvases.size(); ++i) {

            blast::GfxPipelineDesc pipeline_state = {};
            pipeline_state.sc = swapchain;
            for (uint32_t j = 0; j < valid_canvases[i]->draw_elements.size(); ++j) {
                const UIDrawElement& element = valid_canvases[i]->draw_elements[j];

                device->BindConstantBuffer(current_cmd, window_view_ub, 1, window_view_ub->desc.size, 0);
                device->BindConstantBuffer(current_cmd, renderable_ub, 2, renderable_ub->desc.size, 0);

                // 材质参数
                for (auto& sampler_item : element.mi->GetGfxSamplerGroup()) {
                    device->BindSampler(current_cmd, sampler_cache->GetSampler(sampler_item.second), sampler_item.first);
                }

                for (auto& texture_item : element.mi->GetGfxTextureGroup()) {
                    device->BindResource(current_cmd, texture_item.second->GetTexture(), texture_item.first);
                }

                glm::vec4 scissor = element.mi->GetScissor();
                rect.left = scissor.x;
                rect.top = scissor.y;
                rect.right = scissor.z;
                rect.bottom = scissor.w;
                device->BindScissorRects(current_cmd, 1, &rect);

                pipeline_state.vs = element.mi->GetMaterial()->GetVertShader(0, element.vb->GetVertexLayoutType());
                pipeline_state.fs = element.mi->GetMaterial()->GetFragShader(0, element.vb->GetVertexLayoutType());
                pipeline_state.il = vertex_layout_cache->GetVertexLayout(element.vb->GetVertexLayoutType());
                pipeline_state.rs = rasterizer_state_cache->GetRasterizerState(RST_DOUBLESIDED);
                pipeline_state.bs = blend_state_cache->GetDepthStencilState(element.mi->GetMaterial()->GetBlendState());
                pipeline_state.dss = depth_stencil_state_cache->GetDepthStencilState(DSST_UI);

                device->BindPipeline(current_cmd, pipeline_cache->GetPipeline(pipeline_state));

                uint64_t vertex_offsets[] = {0};
                blast::GfxBuffer* vertex_buffers[] = {element.vb->GetHandle()};
                device->BindVertexBuffers(current_cmd, vertex_buffers, 0, 1, vertex_offsets);

                device->BindIndexBuffer(current_cmd, element.ib->GetHandle(), element.ib->GetIndexType(), 0);

                device->DrawIndexed(current_cmd, element.count, element.offset, 0);
            }
        }

        device->RenderPassEnd(current_cmd);
    }
}