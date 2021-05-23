#include "GearEngine.h"
#include "Utility/Log.h"
#include "Scene/Scene.h"
#include "Renderer/Renderer.h"

namespace gear {
    GearEngine::GearEngine() {
        mScene = new Scene();
        mRenderer = new Renderer();
    }

    GearEngine::~GearEngine() {
        SAFE_DELETE(mScene);
        SAFE_DELETE(mRenderer);
    }

    Scene* GearEngine::getScene() {
        return mScene;
    }

    Renderer* GearEngine::getRenderer() {
        return mRenderer;
    }

    GearEngine gEngine;
}