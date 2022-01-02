#pragma once
#include "Component.h"
#include "Math/Math.h"

namespace gear {
    class AnimationInstance;
    class CAnimation : public Component {
    public:
        CAnimation(Entity* entity);

        virtual ~CAnimation();

        static ComponentType GetClassType() { return ComponentType::Animation; }

        ComponentType GetType() override { return ComponentType::Animation; }

        void SetAnimationInstance(AnimationInstance* instance) {
            this->instance = instance;
        }

        void Play();

        void Stop();

        void Replay();

    private:
        AnimationInstance* instance = nullptr;
    };
}