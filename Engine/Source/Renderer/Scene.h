#pragma once
#include <set>

namespace gear {
    class Renderer;
    class View;
    class Entity;
    class Scene {
    public:
        Scene(Renderer* Renderer);

        ~Scene();

        void AddEntity(Entity*);

        void RemoveEntity(Entity*);

    private:
        Renderer* _renderer = nullptr;
        std::set<Entity*> _entities;
    };
}