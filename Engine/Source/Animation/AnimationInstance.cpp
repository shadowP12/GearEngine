#include "AnimationInstance.h"

namespace gear {
    SimpleAnimationInstance::SimpleAnimationInstance() {
    }

    SimpleAnimationInstance::~SimpleAnimationInstance() {
    }

    void SimpleAnimationInstance::SetTime(float time) {
        this->time = time;
        if (!skeleton || !animation_clip) {
            return;
        }
        animation_clip->Sample(time, skeleton);
    }

    void SimpleAnimationInstance::Tick(float dt) {
        if (!skeleton || !animation_clip || puase) {
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
