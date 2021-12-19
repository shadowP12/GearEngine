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
    void Renderer::UpdateShadowMapInfo(Scene* scene, View* view, ShadowMapInfo& shadow_map_info) {
        // 计算相机视锥体在世界坐标的顶点
        glm::vec3 cs_view_frustum_corners[8] = {
                { -1, -1,  -1.0 },
                {  1, -1,  -1.0 },
                { -1,  1,  -1.0 },
                {  1,  1,  -1.0 },
                { -1, -1,  1.0 },
                {  1, -1,  1.0 },
                { -1,  1,  1.0 },
                {  1,  1,  1.0 },
        };

        glm::vec3 ws_view_frustum_vertices[8];
        glm::mat4 projection_view_inverse = glm::inverse(scene->main_camera_info.projection * scene->main_camera_info.view);
        for (uint32_t i = 0; i < 8; i++) {
            ws_view_frustum_vertices[i] = TransformPoint(cs_view_frustum_corners[i], projection_view_inverse);
        }

        for (uint32_t i = 0; i < 4; i++) {
            glm::vec3 dist = ws_view_frustum_vertices[i + 4] - ws_view_frustum_vertices[i];
            ws_view_frustum_vertices[i + 4] = ws_view_frustum_vertices[i] + (dist * shadow_map_info.cs_near_far.x);
            ws_view_frustum_vertices[i] = ws_view_frustum_vertices[i] + (dist * shadow_map_info.cs_near_far.y);
        }

        // 计算投射阴影包围盒与相机视锥体的相交的顶点
        glm::vec3 ws_clipped_shadow_caster_volume[64];
        uint32_t vertex_count = 8;
        for (uint32_t i = 0; i < vertex_count; ++i) {
            ws_clipped_shadow_caster_volume[i] = ws_view_frustum_vertices[i];
        }

        // Get frustum center
        glm::vec3 center = glm::vec3(0.0f);
        for (uint32_t i = 0; i < vertex_count; i++) {
            center += ws_clipped_shadow_caster_volume[i];
        }
        center /= 8.0f;

        float radius = 0.0f;
        for (uint32_t i = 0; i < vertex_count; i++) {
            float distance = glm::length(ws_clipped_shadow_caster_volume[i] - center);
            radius = glm::max(radius, distance);
        }
        radius = std::ceil(radius * 16.0f) / 16.0f;

        // 解决闪烁问题
        // world_units_per_texel表示每一个阴影贴图纹素的世界坐标
        float world_units_per_texel = radius * 2.0f / shadow_map_info.shadow_dimension;
        glm::mat4 shadow_view_matrix = glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0) + scene->light_info.sun_direction, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 shadow_space_origin = TransformPoint(center, shadow_view_matrix);
        glm::vec2 snap_offset(fmod(shadow_space_origin.x, world_units_per_texel), fmod(shadow_space_origin.y, world_units_per_texel));
        shadow_space_origin.x -= snap_offset.x;
        shadow_space_origin.y -= snap_offset.y;
        center = TransformPoint(shadow_space_origin, glm::inverse(shadow_view_matrix));

        glm::vec3 max_extents = glm::vec3(radius);
        glm::vec3 min_extents = -max_extents;

        glm::mat4 light_view_matrix = glm::lookAt(center - scene->light_info.sun_direction * -min_extents.z, center, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 light_model_matrix = glm::inverse(light_view_matrix);
        glm::mat4 light_ortho_matrix = glm::ortho(min_extents.x, max_extents.x, min_extents.y, max_extents.y, 0.0f, max_extents.z - min_extents.z);

        shadow_map_info.light_projection_matrix = light_ortho_matrix;
        shadow_map_info.light_view_matrix = light_view_matrix;
        shadow_map_info.camera_position = GetTranslate(light_model_matrix);
        shadow_map_info.camera_direction = GetAxisZ(light_model_matrix);
    }

    void Renderer::ShadowPass(Scene* scene, View* view) {
        float camera_near = scene->main_camera_info.zn;
        float camera_far = scene->main_camera_info.zf;
        float camera_range = camera_far - camera_near;
        float min_distance = camera_near;
        float max_distance = camera_near + camera_range;
        float cascade_splits[SHADOW_CASCADE_COUNT];

        float pssm_factor = 0.95f;
        float range = max_distance - min_distance;
        float ratio = max_distance / min_distance;
        float log_ratio = glm::clamp(1.0f - pssm_factor, 0.0f, 1.0f);
        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            float distribute = static_cast<float>(i + 1) / SHADOW_CASCADE_COUNT;
            float log_z = static_cast<float>(min_distance * powf(ratio, distribute));
            float uniform_z = min_distance + range * distribute;
            cascade_splits[i] = glm::lerp(uniform_z, log_z, log_ratio);
        }

        // 归一化
        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            cascade_splits[i] = (cascade_splits[i] - camera_near) / camera_range;
        }

        // 更新shadow map的投影矩阵
        float cascade_split = 0.0f;
        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            cascade_shadow_map_infos[i].cs_near_far = { cascade_split, cascade_splits[i] };
            UpdateShadowMapInfo(scene, view, cascade_shadow_map_infos[i]);
            cascade_split = cascade_splits[i];
        }

        // 绘制级联阴影贴图
        uint32_t dc_head = 0;
        uint32_t dc_count = 0;
        for (uint32_t i = 0; i < scene->num_mesh_renderables; ++i) {
            Renderable* rb = &scene->renderables[scene->mesh_renderables[i]];
            for (uint32_t j = 0; j < rb->num_primitives; ++j) {
                RenderPrimitive* rp = &rb->primitives[j];
                if (!rp->cast_shadow) {
                    continue;
                }

                uint32_t dc_idx = dc_head + dc_count;
                uint32_t material_id = rp->mi->GetMaterial()->GetMaterialID();
                uint32_t material_instance_id = rp->mi->GetMaterialInstanceID();
                uint32_t material_variant = 0;
                material_variant |= MaterialVariant::DEPTH;

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
            blast::GfxTextureBarrier barrier[1];
            barrier[0].texture = cascade_shadow_map;
            barrier[0].new_state = blast::RESOURCE_STATE_DEPTH_WRITE;
            device->SetBarrier(current_cmd, 0, nullptr, 1, barrier);
        }

        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            // 设置light view ub
            ViewUniforms vb_storage;
            vb_storage.view_matrix = cascade_shadow_map_infos[i].light_view_matrix;
            vb_storage.proj_matrix = cascade_shadow_map_infos[i].light_projection_matrix;
            device->UpdateBuffer(current_cmd, common_view_ub, &vb_storage, sizeof(ViewUniforms));

            blast::GfxPipelineDesc pipeline_state = {};
            pipeline_state.rp = cascade_shadow_passes[i];
            device->RenderPassBegin(current_cmd, cascade_shadow_passes[i]);

            blast::Viewport viewport;
            viewport.x = 1;
            viewport.y = 1;
            viewport.w = cascade_shadow_map->desc.width - 1;
            viewport.h = cascade_shadow_map->desc.height - 1;
            device->BindViewports(current_cmd, 1, &viewport);

            blast::Rect rect;
            rect.left = 0;
            rect.top = 0;
            rect.right = cascade_shadow_map->desc.width;
            rect.bottom = cascade_shadow_map->desc.height;
            device->BindScissorRects(current_cmd, 1, &rect);

            for (uint32_t i = dc_head; i < dc_count; ++i) {
                uint32_t material_variant = dc_list[i].material_variant;
                uint32_t randerable_id = dc_list[i].renderable_id;
                uint32_t primitive_id = dc_list[i].primitive_id;
                Renderable& renderable = scene->renderables[randerable_id];
                RenderPrimitive& primitive = renderable.primitives[primitive_id];

                device->BindConstantBuffer(current_cmd, common_view_ub, 1, common_view_ub->desc.size, 0);
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
                pipeline_state.rs = rasterizer_state_cache->GetRasterizerState(RST_BACK);
                pipeline_state.bs = blend_state_cache->GetDepthStencilState(BST_OPAQUE);
                pipeline_state.dss = depth_stencil_state_cache->GetDepthStencilState(DSST_SHADOW);

                device->BindPipeline(current_cmd, pipeline_cache->GetPipeline(pipeline_state));

                uint64_t vertex_offsets[] = {0};
                blast::GfxBuffer* vertex_buffers[] = {primitive.vb->GetHandle()};
                device->BindVertexBuffers(current_cmd, vertex_buffers, 0, 1, vertex_offsets);

                device->BindIndexBuffer(current_cmd, primitive.ib->GetHandle(), primitive.ib->GetIndexType(), 0);

                device->DrawIndexed(current_cmd, primitive.count, primitive.offset, 0);
            }
            device->RenderPassEnd(current_cmd);
        }

        // image layout to shader
        {
            blast::GfxTextureBarrier barrier[1];
            barrier[0].texture = cascade_shadow_map;
            barrier[0].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
            device->SetBarrier(current_cmd, 0, nullptr, 1, barrier);
        }

        // 更新view_ub的灯光矩阵参数
        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            view_storage.cascade_splits[i] = cascade_splits[i] * camera_range + camera_near;
            glm::mat4 light_matrix = cascade_shadow_map_infos[i].light_projection_matrix * cascade_shadow_map_infos[i].light_view_matrix;
            view_storage.sun_matrixs[i] = light_matrix;
        }
        device->UpdateBuffer(current_cmd, main_view_ub, &view_storage, sizeof(ViewUniforms));
    }
}