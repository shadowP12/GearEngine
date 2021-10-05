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
        uint32_t _num_renderables = 0;
    };
}