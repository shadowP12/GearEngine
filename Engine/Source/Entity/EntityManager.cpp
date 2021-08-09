#include "EntityManager.h"
#include "Entity.h"

namespace gear {
    EntityManager::EntityManager() {}

    EntityManager::~EntityManager() {
        for (auto entity : _entities) {
            delete entity;
            entity = nullptr;
        }
        _entities.clear();
    }

    Entity* EntityManager::CreateEntity() {
        Entity* entity = new Entity();
        _entities.push_back(entity);
        return entity;
    }

    void EntityManager::DestroyEntity(Entity* entity) {
        for (auto iter = _entities.begin(); iter != _entities.end(); iter++) {
            if (*iter == entity) {
                _entities.erase(iter);
                delete entity;
                entity = nullptr;
                break;
            }
        }
    }
}