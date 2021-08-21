#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include "RenderPipeline/RenderPipeline.h"
#include "Entity/Scene.h"
#include "Entity/EntityManager.h"
#include "Input/InputSystem.h"
#include "MaterialCompiler/MaterialCompiler.h"

namespace gear {
    GearEngine::GearEngine() {
        _renderer = new Renderer();
        _entity_manager = new EntityManager();
        _input_system = new InputSystem();
        _material_compiler = new MaterialCompiler();
    }

    GearEngine::~GearEngine() {
        SAFE_DELETE(_renderer);
        SAFE_DELETE(_entity_manager);
        SAFE_DELETE(_input_system);
        SAFE_DELETE(_material_compiler);
    }

    Renderer* GearEngine::GetRenderer() {
        return _renderer;
    }

    InputSystem* GearEngine::GetInputSystem() {
        return _input_system;
    }

    EntityManager* GearEngine::GetEntityManager() {
        return _entity_manager;
    }

    MaterialCompiler* GearEngine::GetMaterialCompiler() {
        return _material_compiler;
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