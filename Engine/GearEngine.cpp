#include "GearEngine.h"
#include "Utility/Log.h"
#include "Renderer/Renderer.h"

namespace gear {
    GearEngine::GearEngine() {
        mRenderer = new Renderer();
    }

    GearEngine::~GearEngine() {
        SAFE_DELETE(mRenderer);
    }

    Renderer* GearEngine::getRenderer() {
        return mRenderer;
    }

    GearEngine gEngine;
}