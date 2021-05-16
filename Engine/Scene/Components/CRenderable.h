#pragma once
#include "Scene/Components/Component.h"
#include "Math/Math.h"

namespace gear {
    class Entity;
    class CRenderable : public Component {
    public:
        CRenderable(Entity* entity);

        virtual ~CRenderable();

        static ComponentType getClassType() { return ComponentType::Renderable; }

        ComponentType getType() override { return ComponentType::Renderable; }
    };
}