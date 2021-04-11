#pragma once

namespace gear {
    class Renderer;
    class ResourceManager;
    class GearEngine
    {
    public:
        GearEngine();
        ~GearEngine();
        Renderer* getRenderer();
        ResourceManager* getResourceManager();
    private:
        Renderer* mRenderer = nullptr;
        ResourceManager* mResourceManager = nullptr;
    };

    extern GearEngine gEngine;
}