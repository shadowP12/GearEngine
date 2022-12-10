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

        void Play() {
            puase = false;
        }

        void Puase() {
            puase = true;
        }

        void Reset() {
            time = 0.0;
        }

        void SetSpeed(float speed) {
            this->speed = speed;
        }

        float GetSpeed() {
            return speed;
        }

        void SetAnimationMode(AnimationMode mode) {
            this->mode = mode;
        }

        virtual void SetTime(float time) {
            this->time = time;
        }

        float GetTime() {
            return time;
        }

        virtual void Tick(float dt) = 0;

    protected:
        float time = 0.0f;
        float speed = 1.0f;
        bool puase = false;
        AnimationMode mode = AnimationMode::ONCE;
    };

    class SimpleAnimationInstance : public AnimationInstance {
    public:
        SimpleAnimationInstance();

        ~SimpleAnimationInstance();

        void SetSkeleton(std::shared_ptr<Skeleton> skeleton) {
            this->skeleton = skeleton;
        }

        void SetAnimationClip(std::shared_ptr<AnimationClip> animation_clip) {
            this->animation_clip = animation_clip;
        }

        void SetTime(float time) override;

        void Tick(float dt) override;

    private:
        std::shared_ptr<Skeleton> skeleton = nullptr;
        std::shared_ptr<AnimationClip> animation_clip = nullptr;
    };
}