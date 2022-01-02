#pragma once

#include "Math/Math.h"

#include <string>
#include <vector>

namespace gear {

    enum AnimationTarget {
        POSITION,
        ROTATION,
        SCALE
    };

    struct AnimationKey {
        float time;
        glm::vec3 vec_value;
        glm::quat quat_value;
    };

    struct AnimationTrack {
        std::string joint_name;
        AnimationTarget target;
        std::vector<AnimationKey> keys;
    };

    class Skeleton;
    class AnimationClip {
    public:
        AnimationClip(std::vector<AnimationTrack>& tracks, float length) {
            this->tracks = tracks;
            this->length = length;
        }

        ~AnimationClip() {
        }

        float GetAnimationLength() { return length; }

        void Sample(float time, Skeleton* skeleton);

    private:
        friend class AnimationInstance;
        std::vector<AnimationTrack> tracks;
        float length;
    };
}