#pragma once
#include "Component.h"
#include "Math/Math.h"
#include <vector>

namespace gear {
    class Entity;
    class CTransform : public Component {
    public:
        CTransform(Entity* entity);

        virtual ~CTransform();

        static ComponentType GetClassType() { return ComponentType::Transform; }

        ComponentType GetType() override { return ComponentType::Transform; }

        void SetParent(Entity* new_parent);

        Entity* GetParent();

        const std::vector<Entity*>& GetChildren();

        void SetTransform(const glm::mat4& local_transform);

        const glm::mat4& GetTransform();

        const glm::mat4& GetWorldTransform();

    private:
        void UpdateTransform();

    private:
        glm::mat4 _local;
        glm::mat4 _world;
        Entity* _parent = nullptr;
        std::vector<Entity*> _children;
    };
}