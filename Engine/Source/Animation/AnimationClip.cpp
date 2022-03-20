#include "AnimationClip.h"
#include "Skeleton.h"

namespace gear {
    void AnimationClip::Sample(float time, Skeleton* skeleton) {
        glm::clamp(time, 0.0f, length);

        for (uint32_t i = 0; i < tracks.size(); ++i) {
            AnimationTrack& track = tracks[i];

            // TODO: 优化查询速度
            uint32_t idx = 0;
            for (uint32_t j = 0; j < track.keys.size() - 1; ++j) {
                if (time >= track.keys[j].time && time <= track.keys[j + 1].time) {
                    idx = j;
                }
            }

            uint32_t next_idx = idx + 1;
            if (next_idx == track.keys.size()) {
                next_idx = idx;
            }

            //插值计算
            float ratio = 0.0f;
            if (next_idx > idx) {
                ratio = (time - track.keys[idx].time) / (track.keys[next_idx].time - track.keys[idx].time);
            }

            Joint* joint = skeleton->GetJoint(track.joint_name);
            if (!joint) {
                continue;
            }

            if (track.target == AnimationTarget::POSITION) {
                glm::vec3 cur = track.keys[idx].vec_value;
                glm::vec3 next = track.keys[next_idx].vec_value;
                joint->pos = lerp(cur, next, ratio);
            }

            if (track.target == AnimationTarget::ROTATION) {
                glm::quat cur = track.keys[idx].quat_value;
                glm::quat next = track.keys[next_idx].quat_value;
                joint->rot = glm::normalize(glm::slerp(cur, next, ratio));
            }

            if (track.target == AnimationTarget::SCALE) {
                glm::vec3 cur = track.keys[idx].vec_value;
                glm::vec3 next = track.keys[next_idx].vec_value;
                joint->scale = lerp(cur, next, ratio);
            }
        }
    }
}