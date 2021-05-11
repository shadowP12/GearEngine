#pragma once
#include <vector>
#include <map>

namespace gear {
    class Entity;
    class Scene {
    public:
        Scene();
        ~Scene();
        Entity* createEntity();
        void destroyEntity(Entity* entity);
    private:
        std::vector<Entity*> mEntities;
    };
}