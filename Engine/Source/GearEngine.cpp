#include "GearEngine.h"
#include "Renderer/Renderer.h"

namespace gear {
    GearEngine::GearEngine() {
        _renderer = new Renderer();
    }

    GearEngine::~GearEngine() {
        SAFE_DELETE(_renderer);
    }

    Renderer* GearEngine::GetRenderer() {
        return _renderer;
    }

    GearEngine gEngine;
}