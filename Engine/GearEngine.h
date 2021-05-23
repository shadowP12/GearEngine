#pragma once

namespace gear {
    class Scene;
    class Renderer;
    class GearEngine {
    public:
        GearEngine();

        ~GearEngine();

        Scene* getScene();

        Renderer* getRenderer();

    private:
        Scene* mScene = nullptr;
        Renderer* mRenderer = nullptr;
    };

    extern GearEngine gEngine;
}