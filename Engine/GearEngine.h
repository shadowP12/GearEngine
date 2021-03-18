#pragma once

namespace gear {
    class Renderer;
    class GearEngine
    {
    public:
        GearEngine();
        ~GearEngine();
        Renderer* getRenderer();
    private:
        Renderer* mRenderer = nullptr;
    };

    extern GearEngine gEngine;
}