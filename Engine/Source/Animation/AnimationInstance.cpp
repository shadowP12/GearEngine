#include "AnimationInstance.h"

namespace gear {
    SimpleAnimationInstance::SimpleAnimationInstance() {
    }

    SimpleAnimationInstance::~SimpleAnimationInstance() {
    }

    void SimpleAnimationInstance::Tick(float dt) {
        if (!skeleton || !animation_clip) {
            return;
        }

        time += dt;
        if (mode == AnimationMode::LOOP) {
            if (time >= animation_clip->GetAnimationLength()) {
                time = 0.0f;
            }
        }

        animation_clip->Sample(time, skeleton);
    }
}
