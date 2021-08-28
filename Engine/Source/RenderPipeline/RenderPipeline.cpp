#include "RenderPipeline.h"
#include "Entity/Scene.h"
#include "Entity/Entity.h"
#include "Entity/Components/CCamera.h"
#include "Entity/Components/CLight.h"
#include "Entity/Components/CRenderable.h"
#include "Entity/Components/CTransform.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Resource/RenderTarget.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"

#define MAX_NUM_VIEWS 2

namespace gear {
    RenderPipeline::RenderPipeline() {
        _view_ub = new UniformBuffer(sizeof(ViewUniforms));
        _renderable_ub = new UniformBuffer(sizeof(RenderableUniforms) * 1000);
    }

    RenderPipeline::~RenderPipeline() {
        SAFE_DELETE(_view_ub);
        SAFE_DELETE(_renderable_ub);
    }

    void RenderPipeline::SetScene(Scene* scene) {
        _scene = scene;
    }

    void RenderPipeline::Exec() {
        Renderer* renderer = gEngine.GetRenderer();

        if (!_scene) {
            return;
        }
        _num_views = 0;
        _num_lights = 0;
        _num_renderables = 0;

        if (_renderables.size() < _scene->_entities.size()) {
            _renderables.resize(_scene->_entities.size());
        }

        if (_renderable_ub->GetSize() < sizeof(RenderableUniforms) * _scene->_entities.size()) {
            UniformBuffer* old_renderable_ub = _renderable_ub;
            SAFE_DELETE(old_renderable_ub);
            _renderable_ub = new UniformBuffer(sizeof(RenderableUniforms) * _scene->_entities.size());
        }

        if (_lights.size() < _scene->_entities.size()) {
            _lights.resize(_scene->_entities.size());
        }

        if (_views.size() < MAX_NUM_VIEWS) {
            _views.resize(MAX_NUM_VIEWS);
        }

        for (auto entity : _scene->_entities) {
            if (entity->HasComponent<CCamera>()) {
                if (_num_views < MAX_NUM_VIEWS) {
                    CCamera* ccamera = entity->GetComponent<CCamera>();

                    if (ccamera->GetDisplay()) {
                        // 设置display_fb
                        if (ccamera->GetRenderTarget() == nullptr) {
                            _display_fb.clear_value.flags = blast::CLEAR_NONE;
                            _display_fb.width = renderer->GetWidth();
                            _display_fb.height = renderer->GetHeight();
                            std::get<0>(_display_fb.colors[0]) = renderer->GetColor();
                            std::get<1>(_display_fb.colors[0]) = 0;
                            std::get<2>(_display_fb.colors[0]) = 0;
                            std::get<0>(_display_fb.depth_stencil) = renderer->GetDepthStencil();
                            std::get<1>(_display_fb.depth_stencil) = 0;
                            std::get<2>(_display_fb.depth_stencil) = 0;
                        } else {
                            _display_fb.clear_value.flags = blast::CLEAR_NONE;
                            _display_fb.width = ccamera->GetRenderTarget()->GetWidth();
                            _display_fb.height = ccamera->GetRenderTarget()->GetHeight();
                            for (int i = 0; i < TARGET_COUNT; ++i) {
                                auto c = ccamera->GetRenderTarget()->GetColor(i);
                                std::get<0>(_display_fb.colors[i]) = std::get<0>(c)->GetTexture();
                                std::get<1>(_display_fb.colors[i]) = std::get<1>(c);
                                std::get<2>(_display_fb.colors[i]) = std::get<2>(c);
                            }
                            auto d = ccamera->GetRenderTarget()->GetDepthStencil();
                            std::get<0>(_display_fb.depth_stencil) = std::get<0>(d)->GetTexture();
                            std::get<1>(_display_fb.depth_stencil) = std::get<1>(d);
                            std::get<2>(_display_fb.depth_stencil) = std::get<2>(d);
                        }
                        glm::mat4 view_matrix = ccamera->GetViewMatrix();
                        glm::mat4 proj_matrix = ccamera->GetProjMatrix();
                        _view_ub->Update(&view_matrix, offsetof(ViewUniforms, view_matrix), sizeof(glm::mat4));
                        _view_ub->Update(&proj_matrix, offsetof(ViewUniforms, proj_matrix), sizeof(glm::mat4));
                    }

                    if (ccamera->GetMain()) {
                        //
                    }

                    _num_views++;
                }
            }

            if (entity->HasComponent<CLight>()) {
                _num_lights++;
            }

            if (entity->HasComponent<CRenderable>()) {
                CRenderable* crenderable = entity->GetComponent<CRenderable>();
                glm::mat4 model_matrix = entity->GetComponent<CTransform>()->GetWorldTransform();
                glm::mat4 normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
                // 设置renderable
                _renderables[_num_renderables].renderable_ub_size = sizeof(RenderableUniforms);
                _renderables[_num_renderables].renderable_ub_offset = _num_renderables * sizeof(RenderableUniforms);
                _renderables[_num_renderables].renderable_ub = _renderable_ub;
                _renderables[_num_renderables].bone_ub = crenderable->_bb;
                _renderables[_num_renderables].primitives.clear();
                for (uint32_t i = 0; i < crenderable->_primitives.size(); ++i) {
                    _renderables[_num_renderables].primitives.push_back(crenderable->_primitives[i]);
                    BBox& bbox = _renderables[_num_renderables].primitives[i].bbox;
                    bbox.mMin = TransformPoint(bbox.mMin, model_matrix);
                    bbox.mMax = TransformPoint(bbox.mMax, model_matrix);
                }

                // 更新renderable_ub
                _renderable_ub->Update(&model_matrix, _num_renderables * sizeof(RenderableUniforms) + offsetof(RenderableUniforms, model_matrix), sizeof(glm::mat4));
                _renderable_ub->Update(&normal_matrix, _num_renderables * sizeof(RenderableUniforms) + offsetof(RenderableUniforms, normal_matrix), sizeof(glm::mat4));
                _num_renderables++;
            }
        }

        // 若当前场景不存在相机或没有renderable，则不进行渲染
        if (_num_views == 0 || _num_renderables == 0) {
            return;
        }

        // 管线开始前的初始化
        _dc_head = 0;
        renderer->BindFrameUniformBuffer(_view_ub->GetHandle(), _view_ub->GetSize(), 0);

        // common stage
        uint32_t common_dc_head = _dc_head;
        uint32_t num_common_dc = 0;
        for (uint32_t i = 0; i < _num_renderables; ++i) {
            Renderable* rb = &_renderables[i];
            for (uint32_t j = 0; j < rb->primitives.size(); ++j) {
                RenderPrimitive* rp = &rb->primitives[j];
                _dc_list[common_dc_head + num_common_dc].key = 0;
                _dc_list[common_dc_head + num_common_dc].renderable_ub = rb->renderable_ub->GetHandle();
                _dc_list[common_dc_head + num_common_dc].renderable_ub_size = rb->renderable_ub_size;
                _dc_list[common_dc_head + num_common_dc].renderable_ub_offset = rb->renderable_ub_offset;

                if (rb->bone_ub) {
                    _dc_list[common_dc_head + num_common_dc].bone_ub = rb->bone_ub->GetHandle();
                } else {
                    _dc_list[common_dc_head + num_common_dc].bone_ub = nullptr;
                }


                _dc_list[common_dc_head + num_common_dc].vertex_layout = rp->vb->GetVertexLayout();
                _dc_list[common_dc_head + num_common_dc].vb = rp->vb->GetHandle();

                _dc_list[common_dc_head + num_common_dc].ib_count = rp->count;
                _dc_list[common_dc_head + num_common_dc].ib_offset = rp->offset;
                _dc_list[common_dc_head + num_common_dc].ib_type = rp->ib->GetIndexType();
                _dc_list[common_dc_head + num_common_dc].ib = rp->ib->GetHandle();

                _dc_list[common_dc_head + num_common_dc].topo = rp->topo;

                MaterialVariant::Key material_variant = 0;

                _dc_list[common_dc_head + num_common_dc].render_state = rp->mi->GetMaterial()->GetRenderState();

                _dc_list[common_dc_head + num_common_dc].vs = rp->mi->GetMaterial()->GetVertShader(material_variant);
                _dc_list[common_dc_head + num_common_dc].fs = rp->mi->GetMaterial()->GetFragShader(material_variant);

                if (rp->mi->GetUniformBuffer()) {
                    _dc_list[common_dc_head + num_common_dc].material_ub = rp->mi->GetUniformBuffer()->GetHandle();
                    _dc_list[common_dc_head + num_common_dc].material_ub_size = rp->mi->GetUniformBuffer()->GetSize();
                    _dc_list[common_dc_head + num_common_dc].material_ub_offset = 0;
                } else {
                    _dc_list[common_dc_head + num_common_dc].material_ub = nullptr;
                    _dc_list[common_dc_head + num_common_dc].material_ub_size = 0;
                    _dc_list[common_dc_head + num_common_dc].material_ub_offset = 0;
                }

                for (uint32_t k = 0; k < rp->mi->GetGfxSamplerGroup().size(); ++k) {
                    std::get<0>(_dc_list[common_dc_head + num_common_dc].material_samplers[k]) = rp->mi->GetGfxSamplerGroup().at(k).first->GetTexture();
                    std::get<1>(_dc_list[common_dc_head + num_common_dc].material_samplers[k]) = rp->mi->GetGfxSamplerGroup().at(k).second;
                }
                num_common_dc++;
            }
        }
        _dc_head = num_common_dc;

        // 排序
        std::sort(&_dc_list[common_dc_head], &_dc_list[common_dc_head] + num_common_dc);

        renderer->BindFramebuffer(_display_fb);
        for (uint32_t i = common_dc_head; i < num_common_dc; ++i) {
            renderer->ExecuteDrawCall(_dc_list[i]);
        }
        renderer->UnbindFramebuffer();
    }
}