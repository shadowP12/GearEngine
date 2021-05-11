#pragma once
#include "Scene/Components/Component.h"
#include "Math/Math.h"

namespace gear {
    class Entity;
    class CCamera : public Component {
    public:
        CCamera(Entity* entity);
        virtual ~CCamera();
        static ComponentType getClassType() { return ComponentType::Camera; }
        ComponentType getType() override { return ComponentType::Camera; }
    };
}