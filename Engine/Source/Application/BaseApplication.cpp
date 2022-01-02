#include "BaseApplication.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include "Window/BaseWindow.h"
#include "Input/InputSystem.h"
#include "Animation/AnimationSystem.h"

#include <Blast/Gfx/GfxDevice.h>

namespace gear {
    BaseApplication::BaseApplication() {
    }

    BaseApplication::~BaseApplication() {
    }

    void BaseApplication::Run(float dt) {
        Tick(dt);

        gEngine.GetAnimationSystem()->Tick(dt);

        gEngine.GetDevice()->SubmitAllCommandBuffer();
        gEngine.GetRenderer()->RefreshCommandBuffer();

        gear::gEngine.GetInputSystem()->Reset();
    }
}
