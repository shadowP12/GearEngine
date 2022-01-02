#include "CAnimation.h"
#include "Animation/AnimationSystem.h"
#include "Animation/AnimationInstance.h"
#include "Entity/Entity.h"
#include "GearEngine.h"

namespace gear {
    CAnimation::CAnimation(Entity* entity)
            :Component(entity) {
    }

    CAnimation::~CAnimation() {
    }

    void CAnimation::Play() {
        if (!instance) {
            return;
        }

        gEngine.GetAnimationSystem()->RegisterInstance(instance);
    }

    void CAnimation::Stop() {
        if (!instance) {
            return;
        }

        gEngine.GetAnimationSystem()->UnregisterInstance(instance);
    }

    void CAnimation::Replay() {
        if (!instance) {
            return;
        }

        instance->Reset();
        Play();
    }
}