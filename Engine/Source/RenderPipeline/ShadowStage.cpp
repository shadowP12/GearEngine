#include "RenderPipeline.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Resource/RenderTarget.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"

namespace gear {

    static glm::vec4 g_debug_color[SHADOW_CASCADE_COUNT];
    static glm::vec4 g_current_debug_color;

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

    // 计算不同坐标系中的near和far
    glm::vec2 ComputeNearFar(const glm::mat4& view, const glm::vec3* ws_vertices, uint32_t count) {
        glm::vec2 near_far = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() };

        for (uint32_t i = 0; i < count; i++) {
            // 右手坐标系，Z轴朝外，Z值越大越靠近近平面
            float c = TransformPoint(ws_vertices[i], view).z;
            near_far.x = std::max(near_far.x, c);  // near
            near_far.y = std::min(near_far.y, c);  // far
        }
        return near_far;
    }

    glm::vec2 ComputeNearFar(const glm::mat4& view, BBox& ws_shadow_casters_volume) {
        glm::vec3* ws_vertices = ws_shadow_casters_volume.GetCorners().vertices;
        return ComputeNearFar(view, ws_vertices, 8);
    }

    // todo: 后续优化步骤
    void RenderPipeline::ComputeCascadeParams(CascadeParameters& cascade_params) {
        // 方向光取任何位置都没有问题，这里默认设置为相机当前位置
        cascade_params.ws_light_position = _main_camera_info.position;

        // 初始化参数
        cascade_params.ls_near_far = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() };
        cascade_params.vs_near_far = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() };
        cascade_params.ws_shadow_casters_volume = BBox();
        cascade_params.ws_shadow_receivers_volume = BBox();

        // 遍历所有renderable
        for (uint32_t i = 0; i < _num_common_renderables; ++i) {
            Renderable* rb = &_renderables[_common_renderables[i]];
            for (int j = 0; j < rb->primitives.size(); ++j) {
                RenderPrimitive* prim = &rb->primitives[j];
                if (prim->cast_shadow) {
                    cascade_params.ws_shadow_casters_volume.bb_min = glm::min(cascade_params.ws_shadow_casters_volume.bb_min, prim->bbox.bb_min);
                    cascade_params.ws_shadow_casters_volume.bb_max = glm::max(cascade_params.ws_shadow_casters_volume.bb_max, prim->bbox.bb_max);
                }

                if (prim->receive_shadow) {
                    cascade_params.ws_shadow_receivers_volume.bb_min = min(cascade_params.ws_shadow_receivers_volume.bb_min, prim->bbox.bb_min);
                    cascade_params.ws_shadow_receivers_volume.bb_max = max(cascade_params.ws_shadow_receivers_volume.bb_max, prim->bbox.bb_max);
                }
            }
        }

    }

    // 计算视锥体的顶点
    void ComputeFrustumCorners(glm::vec3* out, const glm::mat4& projection_view_inverse, const glm::vec2 cs_near_far) {
        // ndc -> camera -> world
        float near = cs_near_far.x;
        float far = cs_near_far.y;
        glm::vec3 csViewFrustumCorners[8] = {
                { -1, -1,  far },
                {  1, -1,  far },
                { -1,  1,  far },
                {  1,  1,  far },
                { -1, -1,  near },
                {  1, -1,  near },
                { -1,  1,  near },
                {  1,  1,  near },
        };
        for (glm::vec3 c : csViewFrustumCorners) {
            *out++ = TransformPoint(c, projection_view_inverse);
        }
    }

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

    void RenderPipeline::UpdateShadowMapInfo(const CascadeParameters& cascade_params, ShadowMapInfo& shadow_map_info) {
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
        glm::mat4 projection_view_inverse = glm::inverse(_main_camera_info.projection * _main_camera_info.view);
        for (uint32_t i = 0; i < 8; i++) {
            ws_view_frustum_vertices[i] = TransformPoint(cs_view_frustum_corners[i], projection_view_inverse);
        }

        for (uint32_t i = 0; i < 4; i++) {
            glm::vec3 dist = ws_view_frustum_vertices[i + 4] - ws_view_frustum_vertices[i];
            ws_view_frustum_vertices[i + 4] = ws_view_frustum_vertices[i] + (dist * cascade_params.cs_near_far.x);
            ws_view_frustum_vertices[i] = ws_view_frustum_vertices[i] + (dist * cascade_params.cs_near_far.y);
        }

//        ComputeFrustumCorners(ws_view_frustum_vertices, glm::inverse(_main_camera_info.projection * _main_camera_info.view), cascade_params.cs_near_far);

        Frustum frustum(ws_view_frustum_vertices);
        DrawDebugFrustum(frustum, g_current_debug_color);

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
        glm::mat4 shadow_view_matrix = glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0) + _light_info.sun_direction, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 shadow_space_origin = TransformPoint(center, shadow_view_matrix);
        glm::vec2 snap_offset(fmod(shadow_space_origin.x, world_units_per_texel), fmod(shadow_space_origin.y, world_units_per_texel));
        shadow_space_origin.x -= snap_offset.x;
        shadow_space_origin.y -= snap_offset.y;
        center = TransformPoint(shadow_space_origin, glm::inverse(shadow_view_matrix));

        glm::vec3 max_extents = glm::vec3(radius);
        glm::vec3 min_extents = -max_extents;

        glm::mat4 light_view_matrix = glm::lookAt(center - _light_info.sun_direction * -min_extents.z, center, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 light_model_matrix = glm::inverse(light_view_matrix);
        glm::mat4 light_ortho_matrix = glm::ortho(min_extents.x, max_extents.x, min_extents.y, max_extents.y, 0.0f, max_extents.z - min_extents.z);

        shadow_map_info.light_projection_matrix = light_ortho_matrix;
        shadow_map_info.light_view_matrix = light_view_matrix;
        shadow_map_info.camera_position = GetTranslate(light_model_matrix);
        shadow_map_info.camera_direction = GetAxisZ(light_model_matrix);
    }

    void RenderPipeline::ExecShadowStage() {
        Renderer* renderer = gEngine.GetRenderer();

        g_debug_color[0] = glm::vec4(0.0, 1.0, 0.0, 1.0);
        g_debug_color[1] = glm::vec4(0.0, 0.0, 1.0, 1.0);
        g_debug_color[2] = glm::vec4(1.0, 0.0, 0.0, 1.0);

        CascadeParameters cascade_params;
        ComputeCascadeParams(cascade_params);

        float camera_near = _main_camera_info.zn;
        float camera_far = _main_camera_info.zf;
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
            g_current_debug_color = g_debug_color[i];
            cascade_params.cs_near_far = { cascade_split, cascade_splits[i] };
            cascade_split = cascade_splits[i];
            UpdateShadowMapInfo(cascade_params, _cascade_shadow_map_infos[i]);
        }

        // 生成dc
        uint32_t shadow_dc_head = _dc_head;
        uint32_t num_shadow_dc = 0;
        for (uint32_t i = 0; i < _num_common_renderables; ++i) {
            Renderable* rb = &_renderables[_common_renderables[i]];
            for (uint32_t j = 0; j < rb->primitives.size(); ++j) {
                RenderPrimitive* rp = &rb->primitives[j];

                if (!rp->cast_shadow) {
                    continue;
                }

                MaterialVariant::Key material_variant = 0;
                material_variant |= MaterialVariant::DEPTH;

                _dc_list[shadow_dc_head + num_shadow_dc] = {};
                _dc_list[shadow_dc_head + num_shadow_dc].key = 0;
                _dc_list[shadow_dc_head + num_shadow_dc].renderable_ub = rb->renderable_ub->GetHandle();
                _dc_list[shadow_dc_head + num_shadow_dc].renderable_ub_size = rb->renderable_ub_size;
                _dc_list[shadow_dc_head + num_shadow_dc].renderable_ub_offset = rb->renderable_ub_offset;

                if (rb->bone_ub) {
                    material_variant |= MaterialVariant::SKINNING_OR_MORPHING;
                    _dc_list[shadow_dc_head + num_shadow_dc].bone_ub = rb->bone_ub->GetHandle();
                } else {
                    _dc_list[shadow_dc_head + num_shadow_dc].bone_ub = nullptr;
                }

                _dc_list[shadow_dc_head + num_shadow_dc].vertex_layout = rp->vb->GetVertexLayout();
                _dc_list[shadow_dc_head + num_shadow_dc].vb = rp->vb->GetHandle();

                _dc_list[shadow_dc_head + num_shadow_dc].ib_count = rp->count;
                _dc_list[shadow_dc_head + num_shadow_dc].ib_offset = rp->offset;
                _dc_list[shadow_dc_head + num_shadow_dc].ib_type = rp->ib->GetIndexType();
                _dc_list[shadow_dc_head + num_shadow_dc].ib = rp->ib->GetHandle();

                _dc_list[shadow_dc_head + num_shadow_dc].topo = rp->topo;

                _dc_list[shadow_dc_head + num_shadow_dc].render_state.blending_mode = BLENDING_MODE_OPAQUE;
                _dc_list[shadow_dc_head + num_shadow_dc].render_state.cull_mode = blast::CULL_MODE_FRONT;

                _dc_list[shadow_dc_head + num_shadow_dc].vs = rp->mi->GetMaterial()->GetVertShader(material_variant);
                _dc_list[shadow_dc_head + num_shadow_dc].fs = rp->mi->GetMaterial()->GetFragShader(material_variant);

                _dc_list[shadow_dc_head + num_shadow_dc].material_ub = nullptr;
                _dc_list[shadow_dc_head + num_shadow_dc].material_ub_size = 0;
                _dc_list[shadow_dc_head + num_shadow_dc].material_ub_offset = 0;

                num_shadow_dc++;
            }
        }
        _dc_head += num_shadow_dc;

        // 排序
        std::sort(&_dc_list[shadow_dc_head], &_dc_list[shadow_dc_head] + num_shadow_dc);

        // 绘制阴影贴图
        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            _shadow_map_fb.clear_value.flags = blast::CLEAR_DEPTH;
            _shadow_map_fb.clear_value.depth = 1.0f;
            _shadow_map_fb.width = _cascade_shadow_map->GetTexture()->GetWidth();
            _shadow_map_fb.height = _cascade_shadow_map->GetTexture()->GetHeight();
            _shadow_map_fb.viewport[0] = 1;
            _shadow_map_fb.viewport[1] = 1;
            _shadow_map_fb.viewport[2] = _cascade_shadow_map->GetTexture()->GetWidth() - 1;
            _shadow_map_fb.viewport[3] = _cascade_shadow_map->GetTexture()->GetHeight() - 1;
            _shadow_map_fb.depth_stencil.texture = _cascade_shadow_map->GetTexture();
            _shadow_map_fb.depth_stencil.layer = i;
            _shadow_map_fb.depth_stencil.level = 0;

            // 设置light view ub
            _view_ub->Update(&_cascade_shadow_map_infos[i].light_view_matrix, offsetof(ViewUniforms, view_matrix), sizeof(glm::mat4));
            _view_ub->Update(&_cascade_shadow_map_infos[i].light_projection_matrix, offsetof(ViewUniforms, proj_matrix), sizeof(glm::mat4));

            renderer->BindFramebuffer(_shadow_map_fb);
            for (uint32_t i = shadow_dc_head; i < shadow_dc_head + num_shadow_dc; ++i) {
                renderer->ExecuteDrawCall(_dc_list[i]);
            }
            renderer->UnbindFramebuffer();
        }

        // 设置view ub
        // TODO:使用一个批次进行view_ub更新
        glm::vec4 shader_cascade_splits = glm::vec4(std::numeric_limits<float>::max());
        _view_ub->Update(&_display_camera_info.view, offsetof(ViewUniforms, view_matrix), sizeof(glm::mat4));
        _view_ub->Update(&_display_camera_info.projection, offsetof(ViewUniforms, proj_matrix), sizeof(glm::mat4));
        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            shader_cascade_splits[i] = cascade_splits[i] * camera_range + camera_near;
            glm::mat4 light_matrix = _cascade_shadow_map_infos[i].light_projection_matrix * _cascade_shadow_map_infos[i].light_view_matrix;
            _view_ub->Update(&light_matrix, offsetof(ViewUniforms, sun_matrixs[i]), sizeof(glm::mat4));
        }

        _view_ub->Update(&shader_cascade_splits, offsetof(ViewUniforms, cascade_splits), sizeof(glm::vec4));

    }
}