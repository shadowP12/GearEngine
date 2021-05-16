#include "GearEngine.h"
#include "Utility/Log.h"
#include "Scene/Scene.h"
#include "Renderer/Renderer.h"
#include "Resource/ResourceManager.h"

namespace gear {
    GearEngine::GearEngine() {
        mScene = new Scene();
        mRenderer = new Renderer();
        mResourceManager = new ResourceManager();
    }

    GearEngine::~GearEngine() {
        SAFE_DELETE(mScene);
        SAFE_DELETE(mRenderer);
        SAFE_DELETE(mResourceManager);
    }

    Scene* GearEngine::getScene() {
        return mScene;
    }

    Renderer* GearEngine::getRenderer() {
        return mRenderer;
    }

    ResourceManager* GearEngine::getResourceManager() {
        return mResourceManager;
    }

    GearEngine gEngine;
}