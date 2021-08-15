#pragma once

namespace gear {
    class Scene;
    class Renderer;
    class RenderPipeline;
    class EntityManager;
    class GearEngine {
    public:
        GearEngine();

        ~GearEngine();

        EntityManager* GetEntityManager();

        Renderer* GetRenderer();

        RenderPipeline* CreateRenderPipeline();

        void DestroyRenderPipeline(RenderPipeline*);

        Scene* CreateScene();

        void DestroyScene(Scene*);

    private:
        Renderer* _renderer = nullptr;
        EntityManager* _entity_manager = nullptr;
    };

    extern GearEngine gEngine;
}