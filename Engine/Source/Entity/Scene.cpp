#include "Scene.h"
#include "EntityManager.h"
#include "Entity/Entity.h"
#include "GearEngine.h"
#include "Components/CCamera.h"
#include "Components/CLight.h"
#include "Components/CMesh.h"

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

        if (entity->HasComponent<CMesh>()) {
            _num_renderables ++;
        }

        _entities.push_back(entity);
    }

    void Scene::RemoveEntity(Entity* entity) {
        auto iter = find(_entities.begin( ), _entities.end( ), entity);
        if (iter != _entities.end()) {
            if (entity->HasComponent<CMesh>()) {
                _num_renderables --;
            }
            _entities.erase(iter);
        }
    }
}