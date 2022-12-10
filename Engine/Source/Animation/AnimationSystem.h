#pragma once

#include <vector>
#include <memory>

namespace gear {
    class AnimationInstance;

    class AnimationSystem {
    public:
        AnimationSystem();

        ~AnimationSystem();

        void RegisterInstance(std::shared_ptr<AnimationInstance> instance);

        void UnregisterInstance(std::shared_ptr<AnimationInstance> instance);

        void Tick(float dt);

    private:
        std::vector<std::shared_ptr<AnimationInstance>> instances;
    };
}