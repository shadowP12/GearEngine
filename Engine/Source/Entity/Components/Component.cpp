#include "Component.h"
#include "../Entity.h"

namespace gear {
    Component::Component(Entity* entity) {
        _entity = entity;
        _type = ComponentType::None;
    }

    Component::~Component() {
    }
}