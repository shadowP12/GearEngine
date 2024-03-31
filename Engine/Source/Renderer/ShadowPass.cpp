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
    // 线段与三角形相交
    bool IntersectSegmentWithTriangle(glm::vec3& p, glm::vec3 s0, glm::vec3 s1, glm::vec3 t0, glm::vec3 t1, glm::vec3 t2) {
        constexpr const float EPSILON = 1.0f / 65536.0f;  // ~1e-5
        const auto e1 = t1 - t0;
        const auto e2 = t2 - t0;
        const auto d = s1 - s0;
        const auto q = glm::cross(d, e2);
        const auto a = glm::dot(e1, q);
        if (std::abs(a) < EPSILON) {
            // 无法构成三角形
            return false;
        }
        const auto s = s0 - t0;
        const auto u = dot(s, q) * glm::sign(a);
        const auto r = cross(s, e1);
        const auto v = dot(d, r) * glm::sign(a);
        if (u < 0 || v < 0 || u + v > std::abs(a)) {
            // 射线没有和三角形相交
            return false;
        }
        const auto t = dot(e2, r) * glm::sign(a);
        if (t < 0 || t > std::abs(a)) {
            // 射线相交但是不在线段内
            return false;
        }

        // 计算相交点位置
        p = s0 + d * (t / std::abs(a));
        return true;
    }

    // 线段与平面相交
    bool IntersectSegmentWithPlanarQuad(glm::vec3& p, glm::vec3 s0, glm::vec3 s1, glm::vec3 t0, glm::vec3 t1, glm::vec3 t2, glm::vec3 t3) {
        bool hit = IntersectSegmentWithTriangle(p, s0, s1, t0, t1, t2) || IntersectSegmentWithTriangle(p, s0, s1, t0, t2, t3);
        return hit;
    }

    struct SegmentIndex {
        uint8_t v0, v1;
    };

    struct QuadIndex {
        uint8_t v0, v1, v2, v3;
    };

    static SegmentIndex g_box_segments[12] = {
            { 0, 1 }, { 1, 3 }, { 3, 2 }, { 2, 0 },
            { 4, 5 }, { 5, 7 }, { 7, 6 }, { 6, 4 },
            { 0, 4 }, { 1, 5 }, { 3, 7 }, { 2, 6 },
    };

    static QuadIndex g_box_quads[6] = {
            { 2, 0, 1, 3 },  // far
            { 6, 4, 5, 7 },  // near
            { 2, 0, 4, 6 },  // left
            { 3, 1, 5, 7 },  // right
            { 0, 4, 5, 1 },  // bottom
            { 2, 6, 7, 3 },  // top
    };

    // 计算与视锥体相交的点
    void IntersectFrustum(glm::vec3* out_vertices, uint32_t& out_vertex_count, glm::vec3 const* segments_vertices, glm::vec3 const* quads_vertices) {
        for (const SegmentIndex segment : g_box_segments) {
            const glm::vec3 s0{ segments_vertices[segment.v0] };
            const glm::vec3 s1{ segments_vertices[segment.v1] };
            // 每条线段最多与两个quad相交
            uint32_t max_vertex_count = out_vertex_count + 2;
            for (uint32_t j = 0; j < 6 && out_vertex_count < max_vertex_count; ++j) {
                const QuadIndex quad = g_box_quads[j];
                const glm::vec3 t0{ quads_vertices[quad.v0] };
                const glm::vec3 t1{ quads_vertices[quad.v1] };
                const glm::vec3 t2{ quads_vertices[quad.v2] };
                const glm::vec3 t3{ quads_vertices[quad.v3] };
                if (IntersectSegmentWithPlanarQuad(out_vertices[out_vertex_count], s0, s1, t0, t1, t2, t3)) {
                    out_vertex_count++;
                }
            }
        }
    }

    // 计算视锥体与包围盒相交的顶点
    void IntersectFrustumWithBBox(const glm::vec3* ws_frustum_corners, const BBox& ws_bbox, glm::vec3* out_vertices, uint32_t& out_vertex_count) {
        /*
         * 1.计算视锥体处于包围盒的顶点
         * 2.计算包围盒处于视锥体的顶点
         * 3.计算包围盒的边和视锥体平面之间的相交
         * 4.计算视锥体的边和包围盒平面之间的相交
         */

        for (uint32_t i = 0; i < 8; i++) {
            glm::vec3 p = ws_frustum_corners[i];
            out_vertices[out_vertex_count] = p;
            if ((p.x >= ws_bbox.bb_min.x && p.x <= ws_bbox.bb_max.x) &&
                (p.y >= ws_bbox.bb_min.y && p.y <= ws_bbox.bb_max.y) &&
                (p.z >= ws_bbox.bb_min.z && p.z <= ws_bbox.bb_max.z)) {
                out_vertex_count++;
            }
        }
        const bool some_frustum_vertices_are_in_bbox = out_vertex_count > 0;
        constexpr const float EPSILON = 1.0f / 8192.0f; // ~0.012 mm

        // 如果顶点数量等于8说明视锥体被完全覆盖，不需要再进行剩余步骤
        if (out_vertex_count < 8) {
            Frustum frustum(ws_frustum_corners);
            glm::vec4* ws_frustum_planes = frustum.planes;

            // 包围盒的8个顶点
            const glm::vec3* ws_scene_receivers_corners = ws_bbox.GetCorners().vertices;

            for (uint32_t i = 0; i < 8; ++i) {
                glm::vec3 p = ws_scene_receivers_corners[i];
                out_vertices[out_vertex_count] = p;
                // l/b/r/t/f/n分别表示到视锥体对应平面的距离
                float l = glm::dot(glm::vec3(ws_frustum_planes[0]), p) + ws_frustum_planes[0].w;
                float b = glm::dot(glm::vec3(ws_frustum_planes[1]), p) + ws_frustum_planes[1].w;
                float r = glm::dot(glm::vec3(ws_frustum_planes[2]), p) + ws_frustum_planes[2].w;
                float t = glm::dot(glm::vec3(ws_frustum_planes[3]), p) + ws_frustum_planes[3].w;
                float f = glm::dot(glm::vec3(ws_frustum_planes[4]), p) + ws_frustum_planes[4].w;
                float n = glm::dot(glm::vec3(ws_frustum_planes[5]), p) + ws_frustum_planes[5].w;
                if ((l >= EPSILON) && (b >= EPSILON) &&
                    (r >= EPSILON) && (t >= EPSILON) &&
                    (f >= EPSILON) && (n >= EPSILON)) {
                    ++out_vertex_count;
                }
            }

            // 如果视锥体没有被包围盒完全包围，或者没有包围整个包围盒，则说明两者边界存在交点
            if (some_frustum_vertices_are_in_bbox || out_vertex_count < 8) {
                IntersectFrustum(out_vertices, out_vertex_count, ws_scene_receivers_corners, ws_frustum_corners);

                IntersectFrustum(out_vertices, out_vertex_count, ws_frustum_corners, ws_scene_receivers_corners);
            }
        }
    }

    void Renderer::UpdateShadowMapInfo(Scene* scene, View* view, BBox ws_shadow_receivers_volume, ShadowMapInfo& shadow_map_info) {
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
        uint32_t vertex_count = 0;

        IntersectFrustumWithBBox(ws_view_frustum_vertices, ws_shadow_receivers_volume, ws_clipped_shadow_caster_volume, vertex_count);

        // 相交顶点小于等于2时，说明当前没有一个renderable受阴影影响，所以说不需要阴影贴图
        if (vertex_count <= 2) {
            vertex_count = 8;
            for (uint32_t i = 0; i < vertex_count; ++i) {
                ws_clipped_shadow_caster_volume[i] = ws_view_frustum_vertices[i];
            }
        }

        // debug draw
        //view->DrawDebugBox(ws_view_frustum_vertices, glm::vec4(1.0, 0.0, 0.0, 1.0));

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
        // 计算shadow volume
        // TODO: 不应该在每一帧计算阴影体积
        BBox ws_shadow_casters_volume = BBox();
        BBox ws_shadow_receivers_volume = BBox();
        for (uint32_t i = 0; i < scene->num_mesh_renderables; ++i) {
            Renderable* rb = &scene->renderables[scene->mesh_renderables[i]];
            for (uint32_t j = 0; j < rb->num_primitives; ++j) {
                RenderPrimitive* rp = &rb->primitives[j];
                if (rp->cast_shadow) {
                    ws_shadow_casters_volume.bb_min = glm::min(ws_shadow_casters_volume.bb_min, rp->bbox.bb_min);
                    ws_shadow_casters_volume.bb_max = glm::max(ws_shadow_casters_volume.bb_max, rp->bbox.bb_max);
                }

                if (rp->receive_shadow) {
                    ws_shadow_receivers_volume.bb_min = min(ws_shadow_receivers_volume.bb_min, rp->bbox.bb_min);
                    ws_shadow_receivers_volume.bb_max = max(ws_shadow_receivers_volume.bb_max, rp->bbox.bb_max);
                }
            }
        }

        //view->DrawDebugBox(ws_shadow_receivers_volume.GetCorners().vertices, glm::vec4(1.0, 1.0, 0.0, 1.0));

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
            UpdateShadowMapInfo(scene, view, ws_shadow_receivers_volume, cascade_shadow_map_infos[i]);
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

                if (rb->bone_ub && rp->vertex_layout == VLT_SKIN_MESH) {
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

        // Modify image layout to rp
        {
            blast::GfxResourceBarrier barrier[1];
            barrier[0].resource = cascade_shadow_map;
            barrier[0].new_state = blast::RESOURCE_STATE_DEPTH_WRITE;
            device->SetBarrier(current_cmd, 1, barrier);
        }

        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            ViewUniforms vb_storage;
            vb_storage.view_matrix = cascade_shadow_map_infos[i].light_view_matrix;
            vb_storage.proj_matrix = cascade_shadow_map_infos[i].light_projection_matrix;
            UpdateUniformBuffer(current_cmd, common_view_ub, &vb_storage, sizeof(ViewUniforms));

            blast::GfxPipelineDesc pipeline_state = {};
            pipeline_state.rp = cascade_shadow_passes[i];
            device->RenderPassBegin(current_cmd, cascade_shadow_passes[i]);

            device->BindViewport(current_cmd, 0, 0, cascade_shadow_map->width, cascade_shadow_map->height);

            device->BindScissor(current_cmd, 0, 0, cascade_shadow_map->width, cascade_shadow_map->height);

            for (uint32_t i = dc_head; i < dc_count; ++i) {
                uint32_t material_variant = dc_list[i].material_variant;
                uint32_t randerable_id = dc_list[i].renderable_id;
                uint32_t primitive_id = dc_list[i].primitive_id;
                Renderable& renderable = scene->renderables[randerable_id];
                RenderPrimitive& primitive = renderable.primitives[primitive_id];

                device->BindConstantBuffer(current_cmd, common_view_ub, 1, common_view_ub->size, 0);
                device->BindConstantBuffer(current_cmd, renderable.renderable_ub, 2, renderable.renderable_ub_size, renderable.renderable_ub_offset);

                // 骨骼矩阵
                if (material_variant & MaterialVariant::SKINNING_OR_MORPHING) {
                    device->BindConstantBuffer(current_cmd, renderable.bone_ub, 3, renderable.bone_ub->size, 0);
                }

                // 材质参数
                if (primitive.material_ub != nullptr) {
                    device->BindConstantBuffer(current_cmd, primitive.material_ub, 0, primitive.material_ub->size, 0);
                }

                for (auto& sampler_item : primitive.mi->GetGfxSamplerGroup()) {
                    device->BindSampler(current_cmd, sampler_cache->GetSampler(sampler_item.second), sampler_item.first);
                }

                for (auto& texture_item : primitive.mi->GetGfxTextureGroup()) {
                    device->BindResource(current_cmd, texture_item.second.get(), texture_item.first);
                }

                blast::GfxShader* vs = primitive.mi->GetMaterial()->GetVertShader(material_variant, primitive.vertex_layout);
                blast::GfxShader* fs = primitive.mi->GetMaterial()->GetFragShader(material_variant, primitive.vertex_layout);
                if (vs != nullptr && fs != nullptr) {
                    pipeline_state.vs = vs;
                    pipeline_state.fs = fs;
                    pipeline_state.il = vertex_layout_cache->GetVertexLayout(primitive.vertex_layout);
                    pipeline_state.rs = rasterizer_state_cache->GetRasterizerState(RST_BACK);
                    pipeline_state.bs = blend_state_cache->GetDepthStencilState(BST_OPAQUE);
                    pipeline_state.dss = depth_stencil_state_cache->GetDepthStencilState(DSST_SHADOW);

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

        // Modify image layout to shader
        {
            blast::GfxResourceBarrier barrier[1];
            barrier[0].resource = cascade_shadow_map;
            barrier[0].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
            device->SetBarrier(current_cmd, 1, barrier);
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