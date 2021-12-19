#pragma once
#include "Utility/Event.h"
#include "Renderer/RenderData.h"

#include <Blast/Gfx/GfxDefine.h>

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
        bool Prepare(blast::GfxCommandBuffer* cmd);

    private:
        friend class Renderer;
        std::vector<Entity*> entities;
        uint32_t num_mesh_entitys = 0;

        uint32_t num_ui_renderables = 0;
        std::vector<uint32_t> ui_renderables;
        uint32_t num_mesh_renderables = 0;
        std::vector<uint32_t> mesh_renderables;
        std::vector<Renderable> renderables;
        std::vector<RenderableUniforms> renderables_storage;

        CameraInfo display_camera_info;
        CameraInfo main_camera_info;
        LightInfo light_info;

        blast::GfxBuffer* renderables_ub = nullptr;

        blast::GfxTexture* skybox_map = nullptr;
    };
}