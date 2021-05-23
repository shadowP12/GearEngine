#include "Component.h"
#include "Scene/Scene.h"

namespace gear {
    Component::Component(Entity* entity) {
        mEntity = entity;
        mType = ComponentType::None;
    }

    Component::~Component() {
    }
}