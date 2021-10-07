#pragma once
#include "Component.h"
#include "Math/Math.h"

namespace gear {
    class Entity;
    class Texture;
    class CSkybox : public Component {
    public:
        CSkybox(Entity* entity);

        virtual ~CSkybox();

        static ComponentType GetClassType() { return ComponentType::Skybox; }

        ComponentType GetType() override { return ComponentType::Skybox; }

        void SetCubeMap(Texture* cube_map) { _cube_map = cube_map; }

        Texture* GetCubeMap() { return _cube_map; }

    private:
        friend class RenderPipeline;
        Texture* _cube_map;
    };
}