#pragma once
#include "Utility/Event.h"
#include <vector>
#include <map>
#include <string>

namespace gear {
    class Entity;
    class EntityManager {
    public:
        EntityManager();

        ~EntityManager();

        Entity* CreateEntity(const std::string& name = "");

        void DestroyEntity(Entity* entity);

    private:
        std::vector<Entity*> _entities;
        Event<void, Entity*> _on_cmesh_dirty_event;
    };
}