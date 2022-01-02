#include "AnimationSystem.h"
#include "AnimationInstance.h"

namespace gear {
    AnimationSystem::AnimationSystem() {
    }

    AnimationSystem::~AnimationSystem() {
    }

    void AnimationSystem::RegisterInstance(AnimationInstance* instance) {
        for (auto iter = instances.begin(); iter != instances.end(); iter++) {
            if (*iter == instance) {
                return;
            }
        }

        instances.push_back(instance);
    }

    void AnimationSystem::UnregisterInstance(AnimationInstance* instance) {
        for (auto iter = instances.begin(); iter != instances.end(); iter++) {
            if (*iter == instance) {
                instances.erase(iter);
                break;
            }
        }
    }

    void AnimationSystem::Tick(float dt) {
        for (uint32_t i = 0; i < instances.size(); ++i) {
            instances[i]->Tick(dt);
        }
    }
}
