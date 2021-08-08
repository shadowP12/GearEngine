#pragma once

namespace gear {
    class Renderer;

    class GearEngine {
    public:
        GearEngine();

        ~GearEngine();

        Renderer* GetRenderer();

    private:
        Renderer* _renderer = nullptr;
    };

    extern GearEngine gEngine;
}