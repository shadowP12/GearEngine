#include "RenderPipeline.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Resource/RenderTarget.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"

namespace gear {
    void RenderPipeline::ExecUiStage() {
        Renderer* renderer = gEngine.GetRenderer();
        uint32_t ui_dc_head = _dc_head;
        uint32_t num_ui_dc = 0;
        for (uint32_t i = 0; i < _num_ui_renderables; ++i) {
            Renderable* rb = &_renderables[_ui_renderables[i]];
            for (uint32_t j = 0; j < rb->primitives.size(); ++j) {
                RenderPrimitive* rp = &rb->primitives[j];
                _dc_list[ui_dc_head + num_ui_dc].key = 0;
                _dc_list[ui_dc_head + num_ui_dc].renderable_ub = rb->renderable_ub->GetHandle();
                _dc_list[ui_dc_head + num_ui_dc].renderable_ub_size = rb->renderable_ub_size;
                _dc_list[ui_dc_head + num_ui_dc].renderable_ub_offset = rb->renderable_ub_offset;

                if (rb->bone_ub) {
                    _dc_list[ui_dc_head + num_ui_dc].bone_ub = rb->bone_ub->GetHandle();
                } else {
                    _dc_list[ui_dc_head + num_ui_dc].bone_ub = nullptr;
                }

                _dc_list[ui_dc_head + num_ui_dc].vertex_layout = rp->vb->GetVertexLayout();
                _dc_list[ui_dc_head + num_ui_dc].vb = rp->vb->GetHandle();

                _dc_list[ui_dc_head + num_ui_dc].ib_count = rp->count;
                _dc_list[ui_dc_head + num_ui_dc].ib_offset = rp->offset;
                _dc_list[ui_dc_head + num_ui_dc].ib_type = rp->ib->GetIndexType();
                _dc_list[ui_dc_head + num_ui_dc].ib = rp->ib->GetHandle();

                _dc_list[ui_dc_head + num_ui_dc].topo = rp->topo;

                MaterialVariant::Key material_variant = 0;

                _dc_list[ui_dc_head + num_ui_dc].render_state = rp->mi->GetMaterial()->GetRenderState();

                _dc_list[ui_dc_head + num_ui_dc].vs = rp->mi->GetMaterial()->GetVertShader(material_variant);
                _dc_list[ui_dc_head + num_ui_dc].fs = rp->mi->GetMaterial()->GetFragShader(material_variant);

                if (rp->mi->GetUniformBuffer()) {
                    _dc_list[ui_dc_head + num_ui_dc].material_ub = rp->mi->GetUniformBuffer()->GetHandle();
                    _dc_list[ui_dc_head + num_ui_dc].material_ub_size = rp->mi->GetUniformBuffer()->GetSize();
                    _dc_list[ui_dc_head + num_ui_dc].material_ub_offset = 0;
                } else {
                    _dc_list[ui_dc_head + num_ui_dc].material_ub = nullptr;
                    _dc_list[ui_dc_head + num_ui_dc].material_ub_size = 0;
                    _dc_list[ui_dc_head + num_ui_dc].material_ub_offset = 0;
                }

                for (uint32_t k = 0; k < rp->mi->GetGfxSamplerGroup().size(); ++k) {
                    std::get<0>(_dc_list[ui_dc_head + num_ui_dc].material_samplers[k]) = rp->mi->GetGfxSamplerGroup().at(k).first->GetTexture();
                    std::get<1>(_dc_list[ui_dc_head + num_ui_dc].material_samplers[k]) = rp->mi->GetGfxSamplerGroup().at(k).second;
                }
                num_ui_dc++;
            }
        }
        _dc_head += num_ui_dc;

        // 排序
        std::sort(&_dc_list[ui_dc_head], &_dc_list[ui_dc_head] + num_ui_dc);

        renderer->BindFramebuffer(_display_fb);
        for (uint32_t i = ui_dc_head; i < num_ui_dc; ++i) {
            renderer->ExecuteDrawCall(_dc_list[i]);
        }
        renderer->UnbindFramebuffer();
    }
}
