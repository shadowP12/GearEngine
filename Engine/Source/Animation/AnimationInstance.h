#pragma once

#include "Animation/Skeleton.h"
#include "Animation/AnimationClip.h"

namespace gear {
    enum class AnimationMode {
        ONCE,
        LOOP
    };

    class AnimationInstance {
    public:
        AnimationInstance() {}

        ~AnimationInstance() {}

        void Reset() {
            time = 0.0;
        }

        void SetSpeed(float speed) {
            this->speed = speed;
        }

        void SetAnimationMode(AnimationMode mode) {
            this->mode = mode;
        }

        virtual void Tick(float dt) = 0;

    protected:
        float time = 0.0f;
        float speed = 1.0f;
        AnimationMode mode = AnimationMode::ONCE;
    };

    class SimpleAnimationInstance : public AnimationInstance {
    public:
        SimpleAnimationInstance();

        ~SimpleAnimationInstance();

        void SetSkeleton(Skeleton* skeleton) {
            this->skeleton = skeleton;
        }

        void SetAnimationClip(AnimationClip* animation_clip) {
            this->animation_clip = animation_clip;
        }

        void Tick(float dt) override;

    private:
        Skeleton* skeleton = nullptr;
        AnimationClip* animation_clip = nullptr;
    };
}