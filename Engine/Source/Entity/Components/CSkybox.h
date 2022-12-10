#pragma once
#include "Component.h"
#include "Math/Math.h"
#include <GfxDefine.h>

namespace gear {
    class Entity;
    class CSkybox : public Component {
    public:
        CSkybox(Entity* entity);

        virtual ~CSkybox();

        static ComponentType GetClassType() { return ComponentType::Skybox; }

        ComponentType GetType() override { return ComponentType::Skybox; }

        void SetCubeMap(std::shared_ptr<blast::GfxTexture> in_cube_map) { cube_map = in_cube_map; }

        std::shared_ptr<blast::GfxTexture> GetCubeMap() { return cube_map; }

    private:
        std::shared_ptr<blast::GfxTexture> cube_map;
    };
}