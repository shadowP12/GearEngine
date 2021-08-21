#include "Scene.h"
#include "Entity/Entity.h"
#include "Components/CCamera.h"
#include "Components/CLight.h"
#include "Components/CRenderable.h"

namespace gear {
    Scene::Scene() {
    }

    Scene::~Scene() {
    }

    void Scene::AddEntity(Entity* entity) {
        auto iter = find(_entities.begin( ), _entities.end( ), entity);
        if (iter != _entities.end()) {
            return;
        }

        _entities.push_back(entity);
    }

    void Scene::RemoveEntity(Entity* entity) {
        auto iter = find(_entities.begin( ), _entities.end( ), entity);
        if (iter != _entities.end()) {
            _entities.erase(iter);
        }
    }
    
}