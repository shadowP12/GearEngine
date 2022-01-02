#include "Skeleton.h"

namespace gear {
    Skeleton::Skeleton(const std::vector<Joint>& joints) {
        this->joints = joints;

        for (uint32_t i = 0; i < joints.size(); ++i) {
            joint_indices[joints[i].name] = i;
        }
    }

    Skeleton::~Skeleton() {
    }

    Joint* Skeleton::GetJoint(const std::string& name) {
        auto iter = joint_indices.find(name);
        if (iter != joint_indices.end()) {
            return &joints[iter->second];
        }
        return nullptr;
    }
}