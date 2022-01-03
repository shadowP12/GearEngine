#pragma once

#include "Math/Math.h"

#include <Blast/Gfx/GfxDefine.h>

#include <map>
#include <string>
#include <vector>

namespace gear {
    struct Joint {
        std::string name;
        int parent = -1;
        glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::quat rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::mat4 bind_pose = glm::mat4(1.0f);
    };

    class Skeleton {
    public:
        Skeleton(const std::vector<Joint>& joints);

        ~Skeleton();

        Joint* GetJoint(const std::string& name);

    private:
        glm::mat4 GetPoseMatrix(int idx);

        bool Prepare(blast::GfxCommandBuffer* cmd);

        blast::GfxBuffer* GetUniformBuffer() {
            return bone_ub;
        }

    private:
        friend class Scene;
        friend class AnimationClip;
        std::vector<Joint> joints;
        std::map<std::string, uint32_t> joint_indices;
        std::vector<glm::mat4> storage;
        blast::GfxBuffer* bone_ub = nullptr;
    };
}