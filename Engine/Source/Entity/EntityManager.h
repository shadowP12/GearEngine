#pragma once
#include <vector>
#include <map>

namespace gear {
    class Entity;
    class EntityManager {
    public:
        EntityManager();

        ~EntityManager();

        Entity* CreateEntity();

        void DestroyEntity(Entity* entity);

    private:
        void RegisterCamera(Entity* entity);

        void UnregisterCamera(Entity* entity);

    private:
        std::vector<Entity*> _entities;
    };
}