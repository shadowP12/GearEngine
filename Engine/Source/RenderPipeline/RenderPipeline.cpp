#include "RenderPipeline.h"
#include "Entity/Scene.h"
#include "Entity/Entity.h"
#include "Entity/Components/CCamera.h"
#include "Entity/Components/CLight.h"
#include "Entity/Components/CMesh.h"
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

        gear::Texture::Builder cascade_shadow_map_builder;
        cascade_shadow_map_builder.SetWidth(1024);
        cascade_shadow_map_builder.SetHeight(1024);
        cascade_shadow_map_builder.SetLayer(SHADOW_CASCADE_COUNT);
        cascade_shadow_map_builder.SetFormat(blast::FORMAT_D24_UNORM_S8_UINT);
        _cascade_shadow_map = cascade_shadow_map_builder.Build();

        for (uint32_t i = 0; i < SHADOW_CASCADE_COUNT; ++i) {
            _cascade_shadow_map_infos[i].texture_dimension = 1024;
            _cascade_shadow_map_infos[i].shadow_dimension = _cascade_shadow_map_infos[i].texture_dimension - 2;
        }

        _debug_lines.resize(MAX_DEBUG_LINES * 14);

        glm::mat4 identity_matrix = glm::mat4(1.0f);
        _debug_ub = new UniformBuffer(sizeof(RenderableUniforms));
        _debug_ub->Update(&identity_matrix, offsetof(RenderableUniforms, model_matrix), sizeof(glm::mat4));
        _debug_ub->Update(&identity_matrix, offsetof(RenderableUniforms, normal_matrix), sizeof(glm::mat4));

        gear::VertexBuffer::Builder builder;
        builder.SetVertexCount(MAX_DEBUG_POINTS);
        builder.SetAttribute(blast::SEMANTIC_POSITION, blast::FORMAT_R32G32B32_FLOAT);
        builder.SetAttribute(blast::SEMANTIC_COLOR, blast::FORMAT_R32G32B32A32_FLOAT);

        builder.SetVertexCount(MAX_DEBUG_LINES);
        _debug_line_vb = builder.Build();
    }

    RenderPipeline::~RenderPipeline() {
        SAFE_DELETE(_view_ub);
        SAFE_DELETE(_renderable_ub);
        SAFE_DELETE(_debug_ub);
        SAFE_DELETE(_debug_line_vb);
        SAFE_DELETE(_cascade_shadow_map);
    }

    void RenderPipeline::SetScene(Scene* scene) {
        _scene = scene;
    }

    void RenderPipeline::Draw() {
        gEngine.GetRenderer()->EnqueueDisplayTask([this](){ this->Exec(); });
    }

    void RenderPipeline::Exec() {
        Renderer* renderer = gEngine.GetRenderer();

        if (!_scene) {
            return;
        }

        _num_debug_lines = 0;
        _num_views = 0;
        _num_lights = 0;
        _num_renderables = 0;
        _num_ui_renderables = 0;
        _num_common_renderables = 0;

        if (_renderables.size() < _scene->_num_renderables) {
            _renderables.resize(_scene->_num_renderables);
            _ui_renderables.resize(_scene->_num_renderables);
            _common_renderables.resize(_scene->_num_renderables);

            UniformBuffer* old_renderable_ub = _renderable_ub;
            SAFE_DELETE(old_renderable_ub);
            _renderable_ub = new UniformBuffer(sizeof(RenderableUniforms) * _scene->_num_renderables);
        }

        for (auto entity : _scene->_entities) {
            if (entity->HasComponent<CCamera>()) {
                if (_num_views < MAX_NUM_VIEWS) {
                    CCamera* ccamera = entity->GetComponent<CCamera>();

                    if (ccamera->GetDisplay()) {
                        // 设置display_fb
                        if (ccamera->GetRenderTarget() == nullptr) {
                            _display_fb.is_screen_fb = true;
                            _display_fb.clear_value.flags = blast::CLEAR_NONE;
                            _display_fb.width = renderer->GetWidth();
                            _display_fb.height = renderer->GetHeight();
                            _display_fb.viewport[0] = 0.0f;
                            _display_fb.viewport[1] = 0.0f;
                            _display_fb.viewport[2] = renderer->GetWidth();
                            _display_fb.viewport[3] = renderer->GetHeight();
                            _display_fb.colors[0].texture = renderer->GetColor();
                            _display_fb.colors[0].layer = 0;
                            _display_fb.colors[0].level = 0;
                            _display_fb.depth_stencil.texture = renderer->GetDepthStencil();
                            _display_fb.depth_stencil.layer = 0;
                            _display_fb.depth_stencil.level = 0;
                        } else {
                            _display_fb.clear_value.flags = blast::CLEAR_NONE;
                            _display_fb.width = ccamera->GetRenderTarget()->GetWidth();
                            _display_fb.height = ccamera->GetRenderTarget()->GetHeight();
                            _display_fb.viewport[0] = 0.0f;
                            _display_fb.viewport[1] = 0.0f;
                            _display_fb.viewport[2] = ccamera->GetRenderTarget()->GetWidth();
                            _display_fb.viewport[3] = ccamera->GetRenderTarget()->GetHeight();

                            for (int i = 0; i < TARGET_COUNT; ++i) {
                                auto c = ccamera->GetRenderTarget()->GetColor(i);
                                _display_fb.colors[i].texture = std::get<0>(c)->GetTexture();
                                _display_fb.colors[i].layer = std::get<1>(c);
                                _display_fb.colors[i].level = std::get<2>(c);
                            }
                            auto d = ccamera->GetRenderTarget()->GetDepthStencil();
                            _display_fb.depth_stencil.texture = std::get<0>(d)->GetTexture();
                            _display_fb.depth_stencil.layer = std::get<1>(d);
                            _display_fb.depth_stencil.level = std::get<2>(d);
                        }

                        _display_camera_info.zn = ccamera->GetNear();
                        _display_camera_info.zf = ccamera->GetFar();
                        _display_camera_info.model = ccamera->GetModelMatrix();
                        _display_camera_info.view = ccamera->GetViewMatrix();
                        _display_camera_info.projection = ccamera->GetProjMatrix();
                        _display_camera_info.position = GetTranslate(_display_camera_info.model);

                        _view_ub->Update(&_display_camera_info.view, offsetof(ViewUniforms, view_matrix), sizeof(glm::mat4));
                        _view_ub->Update(&_display_camera_info.projection, offsetof(ViewUniforms, proj_matrix), sizeof(glm::mat4));
                    }

                    if (ccamera->GetMain()) {
                        _main_camera_info.zn = ccamera->GetNear();
                        _main_camera_info.zf = ccamera->GetFar();
                        _main_camera_info.model = ccamera->GetModelMatrix();
                        _main_camera_info.view = ccamera->GetViewMatrix();
                        _main_camera_info.projection = ccamera->GetProjMatrix();
                        _main_camera_info.position = GetTranslate(_main_camera_info.model);
                        _view_ub->Update(&_main_camera_info.view, offsetof(ViewUniforms, main_view_matrix), sizeof(glm::mat4));
                    }

                    _num_views++;
                }
            }

            if (entity->HasComponent<CLight>()) {
                _light_info.has_direction_light = true;
                _light_info.sun_direction = entity->GetComponent<CTransform>()->GetFrontVector();
                glm::normalize(_light_info.sun_direction);
                _num_lights++;

                glm::vec4 sun_direction = glm::vec4(_light_info.sun_direction, 1.0f);
                _view_ub->Update(&sun_direction, offsetof(ViewUniforms, sun_direction), sizeof(glm::vec4));
            }

            if (entity->HasComponent<CMesh>()) {
                glm::mat4 model_matrix = entity->GetComponent<CTransform>()->GetWorldTransform();
                glm::mat4 normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
                CMesh* cmesh = entity->GetComponent<CMesh>();

                _renderables[_num_renderables].num_primitives = cmesh->_sub_meshs.size();
                for (uint32_t i = 0; i < cmesh->_sub_meshs.size(); ++i) {
                    // 更新材质信息
                    _renderables[_num_renderables].primitives[i].mi = cmesh->_sub_meshs[i].mi;
                    _renderables[_num_renderables].primitives[i].cast_shadow = cmesh->_sub_meshs[i].cast_shadow;
                    _renderables[_num_renderables].primitives[i].receive_shadow = cmesh->_sub_meshs[i].receive_shadow;
                    _renderables[_num_renderables].primitives[i].material_ub = cmesh->_sub_meshs[i].mi->GetUniformBuffer();

                    // 更新绘制信息
                    _renderables[_num_renderables].primitives[i].ib = cmesh->_sub_meshs[i].ib;
                    _renderables[_num_renderables].primitives[i].vb = cmesh->_sub_meshs[i].vb;
                    _renderables[_num_renderables].primitives[i].topo = cmesh->_sub_meshs[i].topo;
                    _renderables[_num_renderables].primitives[i].count = cmesh->_sub_meshs[i].count;
                    _renderables[_num_renderables].primitives[i].offset = cmesh->_sub_meshs[i].offset;

                    // 更新包围盒
                    _renderables[_num_renderables].primitives[i].bbox = cmesh->_sub_meshs[i].bbox;
                    _renderables[_num_renderables].primitives[i].bbox.bb_min = TransformPoint(_renderables[_num_renderables].primitives[i].bbox.bb_min, model_matrix);
                    _renderables[_num_renderables].primitives[i].bbox.bb_max = TransformPoint(_renderables[_num_renderables].primitives[i].bbox.bb_max, model_matrix);
                }

                // todo：更新bone_ub
                _renderables[_num_renderables].bone_ub = nullptr;

                // 更新renderable_ub
                _renderables[_num_renderables].renderable_ub = _renderable_ub;
                _renderables[_num_renderables].renderable_ub_size = sizeof(RenderableUniforms);
                _renderables[_num_renderables].renderable_ub_offset = _num_renderables * sizeof(RenderableUniforms);
                _renderable_ub->Update(&model_matrix, _num_renderables * sizeof(RenderableUniforms) + offsetof(RenderableUniforms, model_matrix), sizeof(glm::mat4));
                _renderable_ub->Update(&normal_matrix, _num_renderables * sizeof(RenderableUniforms) + offsetof(RenderableUniforms, normal_matrix), sizeof(glm::mat4));

                // 归类
                RenderableType type = cmesh->GetRenderableType();
                if (type == RENDERABLE_COMMON) {
                    _common_renderables[_num_common_renderables] = _num_renderables;
                    _num_common_renderables++;
                } else if (type == RENDERABLE_UI) {
                    _ui_renderables[_num_ui_renderables] = _num_renderables;
                    _num_ui_renderables++;
                }

                _num_renderables++;
            }
        }

        // 若当前场景不存在相机或没有renderable，则不进行渲染
        if (_num_views == 0 || _num_renderables == 0) {
            return;
        }

        // 管线开始前的初始化
        _dc_head = 0;

        // shadow
        ExecShadowStage();

        // base
        ExecBaseStage();

        // debug
        ExecDebugStage();

        // ui
        ExecUiStage();
    }
}