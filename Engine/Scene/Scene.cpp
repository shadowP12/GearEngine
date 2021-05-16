#include "Scene.h"
#include "Entity.h"

namespace gear {
    Scene::Scene() {}

    Scene::~Scene() {}

    Entity* Scene::createEntity() {
        Entity* entity = new Entity();
        mEntities.push_back(entity);
        return entity;
    }

    void Scene::destroyEntity(Entity* entity) {
        for (auto iter = mEntities.begin(); iter != mEntities.end();) {
            if (iter == mEntities.end())
                break;
            if (*iter == entity) {
                mEntities.erase(iter);
                delete entity;
                entity = nullptr;
                break;
            }
        }
    }
}