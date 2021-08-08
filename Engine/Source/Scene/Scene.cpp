#include "Scene.h"
#include "Entity.h"

namespace gear {
    Scene::Scene() {}

    Scene::~Scene() {
        for (auto iter = mEntities.begin(); iter != mEntities.end(); iter++) {
            delete *iter;
            *iter = nullptr;
        }
        mEntities.clear();
    }

    Entity* Scene::createEntity() {
        Entity* entity = new Entity();
        mEntities.push_back(entity);
        return entity;
    }

    void Scene::destroyEntity(Entity* entity) {
        for (auto iter = mEntities.begin(); iter != mEntities.end(); iter++) {
            if (*iter == entity) {
                mEntities.erase(iter);
                delete entity;
                entity = nullptr;
                break;
            }
        }
    }

    void Scene::registerCamera(Entity* entity) {
        for (auto iter = mCameraEntities.begin(); iter != mCameraEntities.end(); iter++) {
            if (*iter == entity) {
                return;
            }
        }
        mCameraEntities.push_back(entity);
    }

    void Scene::unregisterCamera(Entity* entity) {
        for (auto iter = mCameraEntities.begin(); iter != mCameraEntities.end(); iter++) {
            if (*iter == entity) {
                mCameraEntities.erase(iter);
                return;
            }
        }
    }
}