#pragma once
#include "Scene/Components/Component.h"
#include "Math/Math.h"

namespace gear {
    class Entity;
    class CLight : public Component {
    public:
        CLight(Entity* entity);
        virtual ~CLight();
        static ComponentType getClassType() { return ComponentType::Light; }
        ComponentType getType() override { return ComponentType::Light; }
    };
}