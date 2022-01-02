#pragma once

#include <vector>

namespace gear {
    class AnimationInstance;

    class AnimationSystem {
    public:
        AnimationSystem();

        ~AnimationSystem();

        void RegisterInstance(AnimationInstance* instance);

        void UnregisterInstance(AnimationInstance* instance);

        void Tick(float dt);

    private:
        std::vector<AnimationInstance*> instances;
    };
}