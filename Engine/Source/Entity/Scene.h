#pragma once
#include <vector>

namespace gear {
    class Entity;
    class Scene {
    public:
        Scene();

        ~Scene();

        void AddEntity(Entity*);

        void RemoveEntity(Entity*);

    private:
        friend class RenderPipeline;
        std::vector<Entity*> _entities;
        std::vector<Entity*> _light_entities;
        std::vector<Entity*> _camera_entities;
        std::vector<Entity*> _renderable_entities;
    };
}