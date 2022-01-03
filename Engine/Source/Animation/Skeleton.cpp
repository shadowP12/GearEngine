#include "Skeleton.h"
#include "GearEngine.h"

#include <Blast/Gfx/GfxDevice.h>

namespace gear {
    Skeleton::Skeleton(const std::vector<Joint>& joints) {
        this->joints = joints;

        for (uint32_t i = 0; i < joints.size(); ++i) {
            joint_indices[joints[i].name] = i;
        }

        storage.resize(joints.size());

        blast::GfxBufferDesc buffer_desc;
        buffer_desc.size = sizeof(glm::mat4) * joints.size();
        buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
        buffer_desc.res_usage = blast::RESOURCE_USAGE_UNIFORM_BUFFER;
        bone_ub = gEngine.GetDevice()->CreateBuffer(buffer_desc);
    }

    Skeleton::~Skeleton() {
        gEngine.GetDevice()->DestroyBuffer(bone_ub);
    }

    Joint* Skeleton::GetJoint(const std::string& name) {
        auto iter = joint_indices.find(name);
        if (iter != joint_indices.end()) {
            return &joints[iter->second];
        }
        return nullptr;
    }

    glm::mat4 Skeleton::GetPoseMatrix(int idx) {
        glm::mat4 r, t, s;
        r = glm::toMat4(joints[idx].rot);
        t = glm::translate(glm::mat4(1.0), joints[idx].pos);
        s = glm::scale(glm::mat4(1.0), joints[idx].scale);
        glm::mat4 pose_matrix = t * r * s;

        if (joints[idx].parent != -1) {
            pose_matrix = GetPoseMatrix(joints[idx].parent) * pose_matrix;
        }
        return pose_matrix;
    }

    bool Skeleton::Prepare(blast::GfxCommandBuffer* cmd) {
        if (!bone_ub) {
            return false;
        }

        for (uint32_t i = 0; i < joints.size(); ++i) {
            storage[i] = GetPoseMatrix(i) * joints[i].bind_pose;
        }

        gEngine.GetDevice()->UpdateBuffer(cmd, bone_ub, storage.data(), storage.size() * sizeof(glm::mat4));
        return true;
    }
}