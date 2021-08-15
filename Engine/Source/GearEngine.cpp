#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include "RenderPipeline/RenderPipeline.h"
#include "Entity/Scene.h"
#include "Entity/EntityManager.h"

namespace gear {
    GearEngine::GearEngine() {
        _renderer = new Renderer();
        _entity_manager = new EntityManager();
    }

    GearEngine::~GearEngine() {
        SAFE_DELETE(_renderer);
        SAFE_DELETE(_entity_manager);
    }

    Renderer* GearEngine::GetRenderer() {
        return _renderer;
    }

    EntityManager* GearEngine::GetEntityManager() {
        return _entity_manager;
    }

    RenderPipeline* GearEngine::CreateRenderPipeline() {
        return new RenderPipeline();
    }

    void GearEngine::DestroyRenderPipeline(RenderPipeline* render_pipeline) {
        SAFE_DELETE(render_pipeline);
    }

    Scene* GearEngine::CreateScene() {
        return new Scene();
    }

    void GearEngine::DestroyScene(Scene* scene) {
        SAFE_DELETE(scene);
    }

    GearEngine gEngine;
}