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
        friend class RenderScene;
        std::vector<Entity*> mEntities;
    };
}