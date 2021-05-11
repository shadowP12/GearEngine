#pragma once
#include "Scene/Components/Component.h"
#include "Math/Math.h"
#include <vector>

namespace gear {
    class Entity;
    class CTransform : public Component {
    public:
        CTransform(Entity* entity);

        virtual ~CTransform();

        static ComponentType getClassType() { return ComponentType::Transform; }

        ComponentType getType() override { return ComponentType::Transform; }

        void setParent(Entity* newParent);

        Entity* getParent();

        std::vector<Entity*> getChildren();

        void setTransform(const glm::mat4& localTransform);

        void getTransform(const glm::mat4& localTransform);

        const glm::mat4& getTransform();

        const glm::mat4& getWorldTransform();

    private:
        void updateTransform();

    private:
        glm::mat4 mLocal;

        glm::mat4 mWorld;
    };
}