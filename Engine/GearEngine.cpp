#include "GearEngine.h"
#include "Utility/Log.h"
#include "Scene/Scene.h"
#include "Renderer/Renderer.h"
#include "MaterialCompiler/MaterialCompiler.h"

namespace gear {
    GearEngine::GearEngine() {
        mScene = new Scene();
        mRenderer = new Renderer();
        mMaterialCompiler = new MaterialCompiler();
    }

    GearEngine::~GearEngine() {
        SAFE_DELETE(mScene);
        SAFE_DELETE(mRenderer);
        SAFE_DELETE(mMaterialCompiler);
    }

    Scene* GearEngine::getScene() {
        return mScene;
    }

    Renderer* GearEngine::getRenderer() {
        return mRenderer;
    }

    MaterialCompiler* GearEngine::getMaterialCompiler() {
        return mMaterialCompiler;
    }

    GearEngine gEngine;
}