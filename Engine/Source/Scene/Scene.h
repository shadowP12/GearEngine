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
        void registerCamera(Entity* entity);

        void unregisterCamera(Entity* entity);

    private:
        friend class RenderScene;
        friend class CCamera;
        std::vector<Entity*> mEntities;
        std::vector<Entity*> mCameraEntities;
    };
}