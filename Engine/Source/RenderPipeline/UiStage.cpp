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
                uint32_t dc_idx = ui_dc_head + num_ui_dc;
                RenderPrimitive* rp = &rb->primitives[j];
                _dc_list[dc_idx] = {};
                _dc_list[dc_idx].key = 0;
                _dc_list[dc_idx].renderable_ub = rb->renderable_ub->GetHandle();
                _dc_list[dc_idx].renderable_ub_size = rb->renderable_ub_size;
                _dc_list[dc_idx].renderable_ub_offset = rb->renderable_ub_offset;

                if (rb->bone_ub) {
                    _dc_list[dc_idx].bone_ub = rb->bone_ub->GetHandle();
                } else {
                    _dc_list[dc_idx].bone_ub = nullptr;
                }

                _dc_list[dc_idx].vertex_layout = rp->vb->GetVertexLayout();
                _dc_list[dc_idx].vb = rp->vb->GetHandle();

                _dc_list[dc_idx].ib_count = rp->count;
                _dc_list[dc_idx].ib_offset = rp->offset;
                _dc_list[dc_idx].ib_type = rp->ib->GetIndexType();
                _dc_list[dc_idx].ib = rp->ib->GetHandle();

                _dc_list[dc_idx].topo = rp->topo;

                MaterialVariant::Key material_variant = 0;

                _dc_list[dc_idx].render_state = rp->mi->GetMaterial()->GetRenderState();

                _dc_list[dc_idx].vs = rp->mi->GetMaterial()->GetVertShader(material_variant);
                _dc_list[dc_idx].fs = rp->mi->GetMaterial()->GetFragShader(material_variant);

                if (rp->mi->GetUniformBuffer()) {
                    _dc_list[dc_idx].material_ub = rp->mi->GetUniformBuffer()->GetHandle();
                    _dc_list[dc_idx].material_ub_size = rp->mi->GetUniformBuffer()->GetSize();
                    _dc_list[dc_idx].material_ub_offset = 0;
                } else {
                    _dc_list[dc_idx].material_ub = nullptr;
                    _dc_list[dc_idx].material_ub_size = 0;
                    _dc_list[dc_idx].material_ub_offset = 0;
                }

                // 材质采样器
                for (uint32_t k = 0; k < rp->mi->GetGfxSamplerGroup().size(); ++k) {
                    _dc_list[dc_idx].sampler_infos[_dc_list[dc_idx].num_sampler_infos].slot = 4 + k;
                    _dc_list[dc_idx].sampler_infos[_dc_list[dc_idx].num_sampler_infos].texture = rp->mi->GetGfxSamplerGroup().at(k).first->GetTexture();
                    _dc_list[dc_idx].sampler_infos[_dc_list[dc_idx].num_sampler_infos].sampler_desc = rp->mi->GetGfxSamplerGroup().at(k).second;
                    _dc_list[dc_idx].num_sampler_infos++;
                }
                num_ui_dc++;
            }
        }
        _dc_head += num_ui_dc;

        // 排序
        std::sort(&_dc_list[ui_dc_head], &_dc_list[ui_dc_head] + num_ui_dc);

        renderer->BindFramebuffer(_display_fb);
        for (uint32_t i = ui_dc_head; i < ui_dc_head + num_ui_dc; ++i) {
            renderer->ExecuteDrawCall(_dc_list[i]);
        }
        renderer->UnbindFramebuffer();
    }
}
