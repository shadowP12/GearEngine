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
    Renderer::Renderer() {
        device = blast::GfxDevice::CreateDevice();
        shader_compiler = blast::GfxShaderCompiler::CreateShaderCompiler();

        // Initialize current command buffer
        current_cmd = device->RequestCommandBuffer(blast::QueueType::QUEUE_GRAPHICS);

        // 初始化渲染器资源
        {
            blast::GfxBufferDesc buffer_desc = {};
            buffer_desc.size = sizeof(ViewUniforms);
            buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
            buffer_desc.res_usage = blast::RESOURCE_USAGE_UNIFORM_BUFFER;
            main_view_ub = device->CreateBuffer(buffer_desc);
            common_view_ub = device->CreateBuffer(buffer_desc);
            window_view_ub = device->CreateBuffer(buffer_desc);

            buffer_desc.size = sizeof(RenderableUniforms);
            buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
            buffer_desc.res_usage = blast::RESOURCE_USAGE_UNIFORM_BUFFER;
            renderable_ub = device->CreateBuffer(buffer_desc);

            buffer_desc.size = sizeof(AtmosphereParameters);
            buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
            buffer_desc.res_usage = blast::RESOURCE_USAGE_UNIFORM_BUFFER;
            atmosphere_ub = device->CreateBuffer(buffer_desc);

            view_storage.view_matrix = glm::mat4(1.0f);
            view_storage.proj_matrix = glm::mat4(1.0f);
            identity_renderable_storage.model_matrix = glm::mat4(1.0);
            identity_renderable_storage.normal_matrix = glm::mat4(1.0);
        }

        // Shadow
        {
            blast::GfxTextureDesc texture_desc = {};
            texture_desc.width = 1024;
            texture_desc.height = 1024;
            texture_desc.num_layers = SHADOW_CASCADE_COUNT;
            texture_desc.format = blast::FORMAT_D24_UNORM_S8_UINT;
            texture_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
            texture_desc.res_usage = blast::RESOURCE_USAGE_SHADER_RESOURCE | blast::RESOURCE_USAGE_DEPTH_STENCIL;
            // Default depth value 1.0f
            texture_desc.clear.depthstencil.depth = 1.0f;
            cascade_shadow_map = device->CreateTexture(texture_desc);

            for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; ++i) {
                int32_t sub_resource = device->CreateSubresource(cascade_shadow_map, blast::DSV, i, 1, 0, 1);

                blast::GfxRenderPassDesc renderpass_desc = {};
                renderpass_desc.attachments.push_back(
                        blast::RenderPassAttachment::DepthStencil(
                                cascade_shadow_map,
                                sub_resource,
                                blast::LOAD_CLEAR,
                                blast::STORE_STORE
                        )
                );
                cascade_shadow_passes[i] = device->CreateRenderPass(renderpass_desc);
            }
        }

        // Atmoshpere
        {
            blast::GfxTextureDesc texture_desc = {};
            texture_desc.width = TRANSMITTANCE_TEXTURE_WIDTH;
            texture_desc.height = TRANSMITTANCE_TEXTURE_HEIGHT;
            texture_desc.format = blast::FORMAT_R16G16B16A16_FLOAT;
            texture_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
            texture_desc.res_usage = blast::RESOURCE_USAGE_SHADER_RESOURCE | blast::RESOURCE_USAGE_RENDER_TARGET;
            transmittance_lut = device->CreateTexture(texture_desc);

            texture_desc.width = MULTI_SCATTERING_TEXTURE_SIZE;
            texture_desc.height = MULTI_SCATTERING_TEXTURE_SIZE;
            texture_desc.format = blast::FORMAT_R16G16B16A16_FLOAT;
            texture_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
            texture_desc.res_usage = blast::RESOURCE_USAGE_SHADER_RESOURCE | blast::RESOURCE_USAGE_RENDER_TARGET | blast::RESOURCE_USAGE_UNORDERED_ACCESS;
            multi_scatt_texture = device->CreateTexture(texture_desc);

            blast::GfxRenderPassDesc renderpass_desc = {};
            renderpass_desc.attachments.push_back(blast::RenderPassAttachment::RenderTarget(transmittance_lut, -1, blast::LOAD_CLEAR));
            transmittance_rp = device->CreateRenderPass(renderpass_desc);
        }

        // debug
        {
            blast::GfxBufferDesc buffer_desc = {};
            buffer_desc.size = MAX_DEBUG_LINES * 14 * sizeof(float);
            buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
            buffer_desc.res_usage = blast::RESOURCE_USAGE_VERTEX_BUFFER;
            debug_line_vb = device->CreateBuffer(buffer_desc);
        }

        vertex_layout_cache = new VertexLayoutCache(device);
        rasterizer_state_cache = new RasterizerStateCache(device);
        depth_stencil_state_cache = new DepthStencilStateCache(device);
        blend_state_cache = new BlendStateCache(device);
        sampler_cache = new SamplerCache(device);
        renderpass_cache = new RenderPassCache(device);
        pipeline_cache = new PipelineCache(device);
    }

    Renderer::~Renderer() {
        SAFE_DELETE(main_view_ub);
        SAFE_DELETE(common_view_ub);
        SAFE_DELETE(window_view_ub);
        SAFE_DELETE(renderable_ub);
        SAFE_DELETE(atmosphere_ub);

        // Shadow
        SAFE_DELETE(cascade_shadow_map);
        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; ++i) {
            SAFE_DELETE(cascade_shadow_passes[i]);
        }

        // Atmoshpere
        SAFE_DELETE(transmittance_lut);
        SAFE_DELETE(multi_scatt_texture);
        SAFE_DELETE(transmittance_rp);

        // Debug
        SAFE_DELETE(debug_line_vb);

        SAFE_DELETE(vertex_layout_cache);
        SAFE_DELETE(rasterizer_state_cache);
        SAFE_DELETE(depth_stencil_state_cache);
        SAFE_DELETE(blend_state_cache);
        SAFE_DELETE(sampler_cache);
        SAFE_DELETE(renderpass_cache);
        SAFE_DELETE(pipeline_cache);
        SAFE_DELETE(vertex_layout_cache);
        SAFE_DELETE(device);
        SAFE_DELETE(shader_compiler);
    }

    void Renderer::Tick(float dt) {
        device->SubmitAllCommandBuffer();
        current_cmd = device->RequestCommandBuffer(blast::QueueType::QUEUE_GRAPHICS);
    }

    void Renderer::RenderScene(Scene* scene, View* view) {
        if (!scene->Prepare(current_cmd)) {
            return;
        }

        if (!view->Prepare(current_cmd)) {
            return;
        }

        view_storage.view_matrix = scene->display_camera_info.view;
        view_storage.proj_matrix = scene->display_camera_info.projection;
        view_storage.main_view_matrix = scene->main_camera_info.view;
        if (scene->light_info.has_direction_light) {
            view_storage.sun_direction = glm::vec4(scene->light_info.sun_direction, 1.0f);
            view_storage.sun_color_intensity = scene->light_info.sun_color_intensity;
        }
        view_storage.view_position = glm::vec4(scene->display_camera_info.position, 1.0f);
        view_storage.ev100 = scene->main_camera_info.ev100;
        view_storage.exposure = scene->main_camera_info.exposure;
        device->UpdateBuffer(current_cmd, main_view_ub, &view_storage, sizeof(ViewUniforms));

        scene->atmosphere_parameters.resolution = glm::vec4(view->size, 0.0f, 0.0f);
        device->UpdateBuffer(current_cmd, atmosphere_ub, &scene->atmosphere_parameters, sizeof(AtmosphereParameters));

        RenderTransmittanceLut(scene, view);

        RenderMultiScattTexture(scene, view);

        ShadowPass(scene, view);

        BasePass(scene, view);

        PostProcessPass(scene, view);

        DebugPass(scene, view);
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
                if (scene->light_info.has_direction_light && rp->mi->GetMaterial()->GetShadingModel() == SHADING_MODEL_LIT) {
                    material_variant |= MaterialVariant::DIRECTIONAL_LIGHTING;
                }

                if (scene->light_info.has_ibl && rp->mi->GetMaterial()->GetShadingModel() == SHADING_MODEL_LIT) {
                    material_variant |= MaterialVariant::IBL;
                }

                if (rp->receive_shadow) {
                    material_variant |= MaterialVariant::SHADOW_RECEIVER;
                }

                if (rb->bone_ub &&
                    rp->vertex_layout == VLT_SKIN_MESH) {
                    material_variant |= MaterialVariant::SKINNING_OR_MORPHING;
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

        // Remove view matrix's translate
        ViewUniforms vb_storage = view_storage;
        vb_storage.view_matrix = glm::mat4(glm::mat3(vb_storage.view_matrix));
        device->UpdateBuffer(current_cmd, common_view_ub, &vb_storage, sizeof(ViewUniforms));
        device->UpdateBuffer(current_cmd, renderable_ub, &identity_renderable_storage, sizeof(RenderableUniforms));

        // Modify image layout to rp
        {
            blast::GfxResourceBarrier barrier[2];
            barrier[0].resource = view->main_rt;
            barrier[0].new_state = blast::RESOURCE_STATE_RENDERTARGET;
            barrier[1].resource = view->depth_rt;
            barrier[1].new_state = blast::RESOURCE_STATE_DEPTH_WRITE;
            device->SetBarrier(current_cmd, 2, barrier);
        }

        device->RenderPassBegin(current_cmd, view->renderpass);

        device->BindViewport(current_cmd, 0, 0, view->main_rt->width, view->main_rt->height);

        device->BindScissor(current_cmd, 0, 0, view->main_rt->width, view->main_rt->height);

        // Draw skybox
        if (scene->skybox_map && !scene->should_render_atmosphere) {
            device->BindConstantBuffer(current_cmd, common_view_ub, 1, common_view_ub->size, 0);
            device->BindConstantBuffer(current_cmd, renderable_ub, 2, renderable_ub->size, 0);

            device->BindResource(current_cmd, scene->skybox_map, 0);

            blast::GfxSamplerDesc default_sampler = {};
            device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

            blast::GfxBuffer* cube_buffer = gEngine.GetBuiltinResources()->GetCubeBuffer().get();

            blast::GfxShader* vs = gEngine.GetBuiltinResources()->GetSkyBoxMaterial()->GetVertShader(0, VertexLayoutType::VLT_P);
            blast::GfxShader* fs = gEngine.GetBuiltinResources()->GetSkyBoxMaterial()->GetFragShader(0, VertexLayoutType::VLT_P);
            if (vs != nullptr && fs != nullptr) {
                blast::GfxPipelineDesc pipeline_state {};
                pipeline_state.rp = view->renderpass;
                pipeline_state.vs = vs;
                pipeline_state.fs = fs;
                pipeline_state.il = vertex_layout_cache->GetVertexLayout(VertexLayoutType::VLT_P);
                pipeline_state.rs = rasterizer_state_cache->GetRasterizerState(RST_DOUBLESIDED);
                pipeline_state.bs = blend_state_cache->GetDepthStencilState(BST_OPAQUE);
                pipeline_state.dss = depth_stencil_state_cache->GetDepthStencilState(DSST_UI);

                device->BindPipeline(current_cmd, pipeline_cache->GetPipeline(pipeline_state));

                uint64_t vertex_offsets[] = {0};
                blast::GfxBuffer* vertex_buffers[] = {cube_buffer};
                device->BindVertexBuffers(current_cmd, vertex_buffers, 0, 1, vertex_offsets);

                device->Draw(current_cmd, 36, 0);
            }
        }

        for (uint32_t i = dc_head; i < dc_head + dc_count; ++i) {
            uint32_t material_variant = dc_list[i].material_variant;
            uint32_t randerable_id = dc_list[i].renderable_id;
            uint32_t primitive_id = dc_list[i].primitive_id;
            Renderable& renderable = scene->renderables[randerable_id];
            RenderPrimitive& primitive = renderable.primitives[primitive_id];

            device->BindConstantBuffer(current_cmd, main_view_ub, 1, main_view_ub->size, 0);
            device->BindConstantBuffer(current_cmd, renderable.renderable_ub, 2, renderable.renderable_ub_size, renderable.renderable_ub_offset);

            // Material params
            if (primitive.material_ub != nullptr) {
                device->BindConstantBuffer(current_cmd, primitive.material_ub, 0, primitive.material_ub->size, 0);
            }

            // Bone matrix
            if (material_variant & MaterialVariant::SKINNING_OR_MORPHING) {
                device->BindConstantBuffer(current_cmd, renderable.bone_ub, 3, renderable.bone_ub->size, 0);
            }

            for (auto& sampler_item : primitive.mi->GetGfxSamplerGroup()) {
                device->BindSampler(current_cmd, sampler_cache->GetSampler(sampler_item.second), sampler_item.first);
            }

            for (auto& texture_item : primitive.mi->GetGfxTextureGroup()) {
                device->BindResource(current_cmd, texture_item.second.get(), texture_item.first);
            }

            // IBL
            if (material_variant & MaterialVariant::IBL) {
                blast::GfxSamplerDesc ibl_sampler_desc;
                device->BindSampler(current_cmd, sampler_cache->GetSampler(ibl_sampler_desc), 11);
                device->BindResource(current_cmd, scene->light_info.irradiance_map, 11);
                device->BindResource(current_cmd, scene->light_info.prefiltered_map, 12);
                device->BindResource(current_cmd, scene->light_info.lut, 13);
            }

            // Shadow map
            if (material_variant & MaterialVariant::SHADOW_RECEIVER) {
                blast::GfxSamplerDesc shadow_sampler_desc;
                shadow_sampler_desc.min_filter = blast::FILTER_NEAREST;
                shadow_sampler_desc.mag_filter = blast::FILTER_NEAREST;
                shadow_sampler_desc.address_u = blast::ADDRESS_MODE_CLAMP_TO_BORDER;
                shadow_sampler_desc.address_v = blast::ADDRESS_MODE_CLAMP_TO_BORDER;
                shadow_sampler_desc.address_w = blast::ADDRESS_MODE_CLAMP_TO_BORDER;
                device->BindSampler(current_cmd, sampler_cache->GetSampler(shadow_sampler_desc), 10);
                device->BindResource(current_cmd, cascade_shadow_map, 10);
            }

            blast::GfxShader* vs = primitive.mi->GetMaterial()->GetVertShader(material_variant, primitive.vertex_layout);
            blast::GfxShader* fs = primitive.mi->GetMaterial()->GetFragShader(material_variant, primitive.vertex_layout);
            if (vs != nullptr && fs != nullptr) {
                blast::GfxPipelineDesc pipeline_state {};
                pipeline_state.rp = view->renderpass;
                pipeline_state.vs = vs;
                pipeline_state.fs = fs;
                pipeline_state.il = vertex_layout_cache->GetVertexLayout(primitive.vertex_layout);
                pipeline_state.rs = rasterizer_state_cache->GetRasterizerState(RST_DOUBLESIDED);
                pipeline_state.bs = blend_state_cache->GetDepthStencilState(primitive.mi->GetMaterial()->GetBlendState());
                // Handle transparent depth
                if (primitive.mi->GetMaterial()->GetBlendState() == BST_TRANSPARENT) {
                    pipeline_state.dss = depth_stencil_state_cache->GetDepthStencilState(DSST_UI);
                } else {
                    pipeline_state.dss = depth_stencil_state_cache->GetDepthStencilState(DSST_DEFAULT);
                }

                device->BindPipeline(current_cmd, pipeline_cache->GetPipeline(pipeline_state));

                uint64_t vertex_offsets[] = {0};
                blast::GfxBuffer* vertex_buffers[] = {primitive.vb};
                device->BindVertexBuffers(current_cmd, vertex_buffers, 0, 1, vertex_offsets);

                device->BindIndexBuffer(current_cmd, primitive.ib, primitive.index_type, 0);

                device->DrawIndexed(current_cmd, primitive.index_count, 0, 0);
            }
        }
        device->RenderPassEnd(current_cmd);
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

        device->BindViewport(current_cmd, 0.0f, 0.0f, window->GetWidth(), window->GetHeight());

        device->BindScissor(current_cmd, 0, 0, window->GetWidth(), window->GetHeight());

        view_storage.view_matrix = glm::mat4(1.0f);
        view_storage.proj_matrix = glm::mat4(1.0f);
        UpdateUniformBuffer(current_cmd, common_view_ub, &view_storage, sizeof(ViewUniforms));
        UpdateUniformBuffer(current_cmd, renderable_ub, &identity_renderable_storage, sizeof(RenderableUniforms));

        view_storage.view_matrix = glm::mat4(1.0f);
        view_storage.proj_matrix = glm::ortho(0.0f, (float)window->GetWidth(), 0.0f, (float)window->GetHeight(), 0.0f, 10.0f);
        UpdateUniformBuffer(current_cmd, window_view_ub, &view_storage, sizeof(ViewUniforms));

        device->RenderPassBegin(current_cmd, swapchain);

        for (uint32_t i = 0; i < valid_views.size(); ++i) {
            device->BindConstantBuffer(current_cmd, common_view_ub, 1, common_view_ub->size, 0);
            device->BindConstantBuffer(current_cmd, renderable_ub, 2, renderable_ub->size, 0);

            device->BindResource(current_cmd, valid_views[i]->GetOutPostProcessRT(), 0);

            blast::GfxSamplerDesc default_sampler = {};
            device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

            blast::GfxBuffer* quad_buffer = gEngine.GetBuiltinResources()->GetQuadBuffer().get();
            blast::GfxShader* vs = gEngine.GetBuiltinResources()->GetBlitMaterial()->GetVertShader(0, VertexLayoutType::VLT_P_T0);
            blast::GfxShader* fs = gEngine.GetBuiltinResources()->GetBlitMaterial()->GetFragShader(0, VertexLayoutType::VLT_P_T0);
            if (vs != nullptr && fs != nullptr) {
                blast::GfxPipelineDesc pipeline_state = {};
                pipeline_state.sc = swapchain;;
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

        for (uint32_t i = 0; i < valid_canvases.size(); ++i) {
            blast::GfxPipelineDesc pipeline_state = {};
            pipeline_state.sc = swapchain;
            for (uint32_t j = 0; j < valid_canvases[i]->draw_elements.size(); ++j) {
                const UIDrawElement& element = valid_canvases[i]->draw_elements[j];

                device->BindConstantBuffer(current_cmd, window_view_ub, 1, window_view_ub->size, 0);
                device->BindConstantBuffer(current_cmd, renderable_ub, 2, renderable_ub->size, 0);

                // Material params
                for (auto& sampler_item : element.mi->GetGfxSamplerGroup()) {
                    device->BindSampler(current_cmd, sampler_cache->GetSampler(sampler_item.second), sampler_item.first);
                }

                for (auto& texture_item : element.mi->GetGfxTextureGroup()) {
                    device->BindResource(current_cmd, texture_item.second.get(), texture_item.first);
                }

                glm::vec4 scissor = element.mi->GetScissor();
                device->BindScissor(current_cmd, scissor.x, scissor.y, scissor.z, scissor.w);

                blast::GfxShader* vs = element.mi->GetMaterial()->GetVertShader(0, VertexLayoutType::VLT_UI);
                blast::GfxShader* fs = element.mi->GetMaterial()->GetFragShader(0, VertexLayoutType::VLT_UI);
                if (vs != nullptr && fs != nullptr) {
                    pipeline_state.vs = vs;
                    pipeline_state.fs = fs;
                    pipeline_state.il = vertex_layout_cache->GetVertexLayout(VertexLayoutType::VLT_UI);
                    pipeline_state.rs = rasterizer_state_cache->GetRasterizerState(RST_DOUBLESIDED);
                    pipeline_state.bs = blend_state_cache->GetDepthStencilState(element.mi->GetMaterial()->GetBlendState());
                    pipeline_state.dss = depth_stencil_state_cache->GetDepthStencilState(DSST_UI);

                    device->BindPipeline(current_cmd, pipeline_cache->GetPipeline(pipeline_state));

                    uint64_t vertex_offsets[] = {0};
                    blast::GfxBuffer* vertex_buffers[] = {element.vb};
                    device->BindVertexBuffers(current_cmd, vertex_buffers, 0, 1, vertex_offsets);

                    device->BindIndexBuffer(current_cmd, element.ib, element.index_type, 0);

                    device->DrawIndexed(current_cmd, element.index_count, element.index_offset, 0);
                }
            }
        }

        device->RenderPassEnd(current_cmd);
    }

    void Renderer::UpdateUniformBuffer(blast::GfxCommandBuffer* cmd, blast::GfxBuffer* buffer, const void* data, uint64_t size, uint64_t offset) {
        blast::GfxResourceBarrier barrier;
        barrier.resource = buffer;
        barrier.new_state = blast::RESOURCE_STATE_COPY_DEST;
        device->SetBarrier(cmd, 1, &barrier);

        device->UpdateBuffer(cmd, buffer, data, size);

        barrier.resource = buffer;
        barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(cmd, 1, &barrier);
    }

    void Renderer::UpdateVertexBuffer(blast::GfxCommandBuffer* cmd, blast::GfxBuffer* buffer, const void* data, uint64_t size, uint64_t offset) {
        blast::GfxResourceBarrier barrier;
        barrier.resource = buffer;
        barrier.new_state = blast::RESOURCE_STATE_COPY_DEST;
        device->SetBarrier(cmd, 1, &barrier);

        device->UpdateBuffer(cmd, buffer, data, size);

        barrier.resource = buffer;
        barrier.new_state = blast::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        device->SetBarrier(cmd, 1, &barrier);
    }

    void Renderer::UpdateIndexBuffer(blast::GfxCommandBuffer* cmd, blast::GfxBuffer* buffer, const void* data, uint64_t size, uint64_t offset) {
        blast::GfxResourceBarrier barrier;
        barrier.resource = buffer;
        barrier.new_state = blast::RESOURCE_STATE_COPY_DEST;
        device->SetBarrier(cmd, 1, &barrier);

        device->UpdateBuffer(cmd, buffer, data, size);

        barrier.resource = buffer;
        barrier.new_state = blast::RESOURCE_STATE_INDEX_BUFFER;
        device->SetBarrier(cmd, 1, &barrier);
    }
}