#include "GearEngine.h"
#include "Utility/Log.h"
#include "Renderer/Renderer.h"
#include "Resource/ResourceManager.h"

namespace gear {
    GearEngine::GearEngine() {
        mRenderer = new Renderer();
        mResourceManager = new ResourceManager();
    }

    GearEngine::~GearEngine() {
        SAFE_DELETE(mRenderer);
        SAFE_DELETE(mResourceManager);
    }

    Renderer* GearEngine::getRenderer() {
        return mRenderer;
    }

    ResourceManager* GearEngine::getResourceManager() {
        return mResourceManager;
    }

    GearEngine gEngine;
}