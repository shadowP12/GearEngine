#pragma once
#include "Component.h"
#include "Math/Math.h"

namespace gear {
    class Entity;
    class CLight : public Component {
    public:
        CLight(Entity* entity);

        virtual ~CLight();

        static ComponentType GetClassType() { return ComponentType::Light; }

        ComponentType GetType() override { return ComponentType::Light; }
    };
}