#include "Entity.h"

namespace gear {
    Entity::Entity(const std::string& name) {
        _name = name;
    }

    Entity::~Entity() {
        for (int i = 0; i < _components.size(); ++i) {
            delete _components[i];
            _components[i] = nullptr;
        }
        _components.clear();
    }
}