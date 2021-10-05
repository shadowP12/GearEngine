#include "Scene.h"
#include "Entity/Entity.h"
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
            CMesh* cmesh = entity->GetComponent<CMesh>();
            _num_renderables += cmesh->_sub_meshs.size();
        }

        _entities.push_back(entity);
    }

    void Scene::RemoveEntity(Entity* entity) {
        auto iter = find(_entities.begin( ), _entities.end( ), entity);
        if (iter != _entities.end()) {
            if (entity->HasComponent<CMesh>()) {
                CMesh* cmesh = entity->GetComponent<CMesh>();
                _num_renderables -= cmesh->_sub_meshs.size();
            }
            _entities.erase(iter);
        }
    }
    
}