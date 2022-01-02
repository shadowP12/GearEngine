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
        AnimationClip(std::vector<AnimationTrack>& tracks) {
            this->tracks = tracks;
            for (uint32_t i = 0; i < tracks.size(); ++i) {
                for (uint32_t j = 0; j < tracks[i].keys.size(); ++j) {
                    if (tracks[i].keys[j].time >= length) {
                        length = tracks[i].keys[j].time;
                    }
                }
            }
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