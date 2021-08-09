#include "Scene.h"
#include "Renderer"
#include "Entity/Entity.h"
namespace gear {
    Scene::Scene(Renderer* renderer) {
        _renderer = renderer;
    }

    Scene::~Scene() {
    }

    void Scene::AddEntity(Entity* entity) {
        _entities.insert(entity);
    }

    void Scene::RemoveEntity(Entity* entity) {
        _entities.erase(entity);
    }

    
}