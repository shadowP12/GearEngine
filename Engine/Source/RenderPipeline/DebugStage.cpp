#include "RenderPipeline.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Resource/RenderTarget.h"
#include "Resource/BuiltinResources.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
namespace gear {
    void RenderPipeline::AddDebugLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& c) {
        if (!_enable_debug) {
            return;
        }

        _debug_lines[_num_debug_lines * 14 + 0] = p0.x;
        _debug_lines[_num_debug_lines * 14 + 1] = p0.y;
        _debug_lines[_num_debug_lines * 14 + 2] = p0.z;
        _debug_lines[_num_debug_lines * 14 + 3] = c.x;
        _debug_lines[_num_debug_lines * 14 + 4] = c.y;
        _debug_lines[_num_debug_lines * 14 + 5] = c.z;
        _debug_lines[_num_debug_lines * 14 + 6] = c.w;
        _debug_lines[_num_debug_lines * 14 + 7] = p1.x;
        _debug_lines[_num_debug_lines * 14 + 8] = p1.y;
        _debug_lines[_num_debug_lines * 14 + 9] = p1.z;
        _debug_lines[_num_debug_lines * 14 + 10] = c.x;
        _debug_lines[_num_debug_lines * 14 + 11] = c.y;
        _debug_lines[_num_debug_lines * 14 + 12] = c.z;
        _debug_lines[_num_debug_lines * 14 + 13] = c.w;
        _num_debug_lines++;
    }

    void RenderPipeline::DrawDebugBox(const BBox& bbox, const glm::vec4& c) {
        if (!_enable_debug) {
            return;
        }

        glm::vec3* corners = bbox.GetCorners().vertices;
        AddDebugLine(corners[0], corners[1], c);
        AddDebugLine(corners[1], corners[3], c);
        AddDebugLine(corners[3], corners[2], c);
        AddDebugLine(corners[2], corners[0], c);

        AddDebugLine(corners[4], corners[5], c);
        AddDebugLine(corners[5], corners[7], c);
        AddDebugLine(corners[7], corners[6], c);
        AddDebugLine(corners[6], corners[4], c);

        AddDebugLine(corners[0], corners[4], c);
        AddDebugLine(corners[1], corners[5], c);
        AddDebugLine(corners[2], corners[6], c);
        AddDebugLine(corners[3], corners[7], c);
    }

    void RenderPipeline::DrawDebugFrustum(const Frustum& frustum, const glm::vec4& c) {
        if (!_enable_debug) {
            return;
        }

        AddDebugLine(frustum.corners[0], frustum.corners[1], c);
        AddDebugLine(frustum.corners[1], frustum.corners[3], c);
        AddDebugLine(frustum.corners[3], frustum.corners[2], c);
        AddDebugLine(frustum.corners[2], frustum.corners[0], c);

        AddDebugLine(frustum.corners[4], frustum.corners[5], c);
        AddDebugLine(frustum.corners[5], frustum.corners[7], c);
        AddDebugLine(frustum.corners[7], frustum.corners[6], c);
        AddDebugLine(frustum.corners[6], frustum.corners[4], c);

        AddDebugLine(frustum.corners[0], frustum.corners[4], c);
        AddDebugLine(frustum.corners[1], frustum.corners[5], c);
        AddDebugLine(frustum.corners[2], frustum.corners[6], c);
        AddDebugLine(frustum.corners[3], frustum.corners[7], c);
    }

    void RenderPipeline::ExecDebugStage() {
        if (!_enable_debug) {
            return;
        }

        Renderer* renderer = gEngine.GetRenderer();
        BuiltinResources* builtin_resources = gEngine.GetBuiltinResources();

        if (_num_debug_lines > 0) {
            _debug_line_vb->Update(_debug_lines.data(), 0, _num_debug_lines * sizeof(float) * 14);

            renderer->BindFramebuffer(_display_fb);

            renderer->ResetUniformBufferSlot();
            renderer->BindFrameUniformBuffer(_view_ub->GetHandle(), _view_ub->GetSize(), 0);
            renderer->BindRenderableUniformBuffer(_debug_ub->GetHandle(), _debug_ub->GetSize(), 0);

            renderer->ResetSamplerSlot();

            renderer->BindVertexShader(builtin_resources->GetDebugMaterial()->GetVertShader(0));
            renderer->BindFragmentShader(builtin_resources->GetDebugMaterial()->GetFragShader(0));

            renderer->SetDepthState(true);
            renderer->SetBlendingMode(BLENDING_MODE_OPAQUE);
            renderer->SetFrontFace(blast::FRONT_FACE_CCW);
            renderer->SetCullMode(blast::CULL_MODE_BACK);
            renderer->SetPrimitiveTopo(blast::PRIMITIVE_TOPO_LINE_LIST);

            renderer->BindVertexBuffer(_debug_line_vb->GetHandle(), _debug_line_vb->GetVertexLayout(), _debug_line_vb->GetSize(), 0);

            renderer->Draw(_num_debug_lines * 2, 0);

            renderer->UnbindFramebuffer();
        }
    }
}
