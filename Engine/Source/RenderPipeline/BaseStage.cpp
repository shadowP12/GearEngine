#include "RenderPipeline.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Resource/RenderTarget.h"
#include "Resource/BuiltinResources.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
namespace gear {
    void RenderPipeline::ExecBaseStage() {
        Renderer* renderer = gEngine.GetRenderer();

        // 排序
        uint32_t common_dc_head = 0;
        uint32_t num_common_dc = 0;
        for (uint32_t i = 0; i < _num_common_renderables; ++i) {
            Renderable* rb = &_renderables[_common_renderables[i]];
            for (uint32_t j = 0; j < rb->num_primitives; ++j) {
                RenderPrimitive* rp = &rb->primitives[j];
                uint32_t dc_idx = common_dc_head + num_common_dc;
                uint32_t material_id = rp->mi->GetMaterial()->GetMaterialID();
                uint32_t material_instance_id = rp->mi->GetMaterialInstanceID();
                uint32_t material_variant = 0;
                if (_light_info.has_direction_light) {
                    material_variant |= MaterialVariant::DIRECTIONAL_LIGHTING;
                }

                if (rp->receive_shadow) {
                    material_variant |= MaterialVariant::SHADOW_RECEIVER;
                }

                _dc_list[dc_idx] = {};
                _dc_list[dc_idx].renderable_id = _common_renderables[i];
                _dc_list[dc_idx].primitive_id = j;
                _dc_list[dc_idx].material_variant = material_variant;
                _dc_list[dc_idx].key |= DrawCall::GenMaterialKey(material_id, material_variant, material_instance_id);

                num_common_dc++;
            }
        }
        std::sort(&_dc_list[common_dc_head], &_dc_list[common_dc_head] + num_common_dc);

        // 绘制
        renderer->BindFramebuffer(_display_fb);
        for (uint32_t i = common_dc_head; i < common_dc_head + num_common_dc; ++i) {
            uint32_t material_variant = _dc_list[i].material_variant;
            uint32_t randerable_id = _dc_list[i].renderable_id;
            uint32_t primitive_id = _dc_list[i].primitive_id;
            Renderable& renderable = _renderables[randerable_id];
            RenderPrimitive& primitive = renderable.primitives[primitive_id];

            renderer->ResetUniformBufferSlot();
            renderer->BindFrameUniformBuffer(_view_ub->GetHandle(), _view_ub->GetSize(), 0);
            renderer->BindRenderableUniformBuffer(renderable.renderable_ub->GetHandle(), renderable.renderable_ub_size, renderable.renderable_ub_offset);
            if (primitive.material_ub != nullptr) {
                blast::GfxBuffer* material_ub = primitive.material_ub->GetHandle();
                renderer->BindMaterialUniformBuffer(material_ub, material_ub->GetSize(), 0);
            }

            renderer->ResetSamplerSlot();

            // 阴影贴图
            if (material_variant & MaterialVariant::SHADOW_RECEIVER) {
                blast::GfxSamplerDesc shadow_sampler_desc;
                shadow_sampler_desc.min_filter = blast::FILTER_NEAREST;
                shadow_sampler_desc.mag_filter = blast::FILTER_NEAREST;
                shadow_sampler_desc.address_u = blast::ADDRESS_MODE_CLAMP_TO_EDGE;
                shadow_sampler_desc.address_v = blast::ADDRESS_MODE_CLAMP_TO_EDGE;
                shadow_sampler_desc.address_w = blast::ADDRESS_MODE_CLAMP_TO_EDGE;

                SamplerInfo shadow_sampler_info = {};
                shadow_sampler_info.slot = 0;
                shadow_sampler_info.layer = 0;
                shadow_sampler_info.num_layers = SHADOW_CASCADE_COUNT;
                shadow_sampler_info.texture = _cascade_shadow_map->GetTexture();
                shadow_sampler_info.sampler_desc = shadow_sampler_desc;

                renderer->BindSampler(shadow_sampler_info);
            }

            // 材质贴图
            for (uint32_t k = 0; k < primitive.mi->GetGfxSamplerGroup().size(); ++k) {
                SamplerInfo material_sampler_info = {};
                material_sampler_info.slot = 4 + k;
                material_sampler_info.layer = 0;
                material_sampler_info.num_layers = 1;
                material_sampler_info.texture = primitive.mi->GetGfxSamplerGroup().at(k).first->GetTexture();
                material_sampler_info.sampler_desc = primitive.mi->GetGfxSamplerGroup().at(k).second;

                renderer->BindSampler(material_sampler_info);
            }

            renderer->BindVertexShader(primitive.mi->GetMaterial()->GetVertShader(material_variant));
            renderer->BindFragmentShader(primitive.mi->GetMaterial()->GetFragShader(material_variant));

            const RenderState& render_state = primitive.mi->GetMaterial()->GetRenderState();
            renderer->SetDepthState(true);
            renderer->SetBlendingMode(render_state.blending_mode);
            renderer->SetFrontFace(blast::FRONT_FACE_CCW);
            renderer->SetCullMode(blast::CULL_MODE_BACK);
            renderer->SetPrimitiveTopo(primitive.topo);

            renderer->BindVertexBuffer(primitive.vb->GetHandle(), primitive.vb->GetVertexLayout(), primitive.vb->GetSize(), 0);
            renderer->BindIndexBuffer(primitive.ib->GetHandle(), primitive.ib->GetIndexType(), primitive.ib->GetSize(), 0);

            renderer->DrawIndexed(primitive.count, primitive.offset);
        }
        renderer->UnbindFramebuffer();
    }
}