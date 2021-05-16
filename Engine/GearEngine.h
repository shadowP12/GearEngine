#pragma once

namespace gear {
    class Scene;
    class Renderer;
    class ResourceManager;
    class GearEngine {
    public:
        GearEngine();

        ~GearEngine();

        Scene* getScene();

        Renderer* getRenderer();

        ResourceManager* getResourceManager();
    private:
        Scene* mScene = nullptr;
        Renderer* mRenderer = nullptr;
        ResourceManager* mResourceManager = nullptr;
    };

    extern GearEngine gEngine;
}