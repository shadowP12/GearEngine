#include "RenderPipeline.h"
#include "Entity/Scene.h"
#include "Entity/Entity.h"
#include "Entity/Components/CCamera.h"
#include "Entity/Components/CLight.h"
#include "Entity/Components/CRenderable.h"

#define MAX_NUM_VIEWS 1

namespace gear {
    RenderPipeline::RenderPipeline() {

    }

    RenderPipeline::~RenderPipeline() {

    }

    void RenderPipeline::SetScene(Scene* scene) {
        _scene = scene;
    }

    void RenderPipeline::Exec() {
        if (!_scene) {
            return;
        }
        _num_views = 0;
        _num_lights = 0;
        _num_renderables = 0;

        if (_renderables.size() < _scene->_entities.size()) {
            _renderables.resize(_scene->_entities.size());
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
                    _num_views++;
                }
            }

            if (entity->HasComponent<CLight>()) {
                _num_lights++;
            }

            if (entity->HasComponent<CRenderable>()) {
                _num_renderables++;
            }
        }

        // 若当前场景不存在相机，则不进行渲染
        if (_num_views == 0) {
            return;
        }



    }
}