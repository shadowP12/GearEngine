#include "BaseApplication.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include "Window/BaseWindow.h"
#include "Input/InputSystem.h"
#include "Animation/AnimationSystem.h"

namespace gear {
    BaseApplication::BaseApplication() {
    }

    BaseApplication::~BaseApplication() {
    }

    void BaseApplication::Run(float dt) {
        Tick(dt);

        gEngine.GetAnimationSystem()->Tick(dt);

        gEngine.GetRenderer()->Tick(dt);

        gear::gEngine.GetInputSystem()->Reset();
    }
}
