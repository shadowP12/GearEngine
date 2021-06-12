#pragma once

namespace gear {
    class Scene;
    class Renderer;
    class MaterialCompiler;
    class GearEngine {
    public:
        GearEngine();

        ~GearEngine();

        Scene* getScene();

        Renderer* getRenderer();

        MaterialCompiler* getMaterialCompiler();

    private:
        Scene* mScene = nullptr;
        Renderer* mRenderer = nullptr;
        MaterialCompiler* mMaterialCompiler = nullptr;
    };

    extern GearEngine gEngine;
}