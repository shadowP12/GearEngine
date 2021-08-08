#pragma once

namespace gear {
    class Scene;
    class Renderer;
    class MaterialCompiler;
    class InputSystem;

    class GearEngine {
    public:
        GearEngine();

        ~GearEngine();

        Scene* getScene();

        Renderer* getRenderer();

        MaterialCompiler* getMaterialCompiler();

        InputSystem* getInputSystem();

    private:
        Scene* mScene = nullptr;
        Renderer* mRenderer = nullptr;
        MaterialCompiler* mMaterialCompiler = nullptr;
        InputSystem* mInputSystem = nullptr;
    };

    extern GearEngine gEngine;
}