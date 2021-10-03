#pragma once

namespace gear {
    class Scene;
    class Renderer;
    class RenderPipeline;
    class EntityManager;
    class InputSystem;
    class MaterialCompiler;
    class BuiltinResources;
    class RenderUtility;

    class GearEngine {
    public:
        GearEngine();

        ~GearEngine();

        InputSystem* GetInputSystem();

        EntityManager* GetEntityManager();

        Renderer* GetRenderer();

        MaterialCompiler* GetMaterialCompiler();

        BuiltinResources* GetBuiltinResources();

        RenderUtility* GetRenderUtility();

        RenderPipeline* CreateRenderPipeline();

        void DestroyRenderPipeline(RenderPipeline*);

        Scene* CreateScene();

        void DestroyScene(Scene*);

    private:
        Renderer* _renderer = nullptr;
        EntityManager* _entity_manager = nullptr;
        InputSystem* _input_system = nullptr;
        MaterialCompiler* _material_compiler = nullptr;
        BuiltinResources* _builtin_resources = nullptr;
        RenderUtility* _render_utility = nullptr;
    };

    extern GearEngine gEngine;
}