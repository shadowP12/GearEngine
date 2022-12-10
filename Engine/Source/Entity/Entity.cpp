#include "Entity.h"

namespace gear {
    Entity::Entity(const std::string &name) {
        _name = name;
    }

    Entity::~Entity() {
        for (int i = 0; i < _components.size(); ++i) {
            delete _components[i];
            _components[i] = nullptr;
        }
        _components.clear();
    }

    std::shared_ptr<Entity> Entity::Create(const std::string &name) {
        return std::make_shared<Entity>(name);
    }

    std::string Entity::GetName() {
        return _name;
    }
}