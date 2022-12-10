#pragma once
#include "Utility/Event.h"
#include "Renderer/RenderData.h"

#include <GfxDefine.h>

#include <vector>

namespace gear {
    class Entity;
    class Scene {
    public:
        Scene(const std::string& name);

        ~Scene();

        void AddEntity(std::shared_ptr<Entity>);

        void RemoveEntity(std::shared_ptr<Entity>);

        static std::shared_ptr<Scene> Create(const std::string& name);

    private:
        bool Prepare(blast::GfxCommandBuffer* cmd);

    private:
        friend class Renderer;
        std::string name;
        std::vector<std::shared_ptr<Entity>> entities;

        // Render
        uint32_t num_mesh_entitys = 0;
        uint32_t num_mesh_renderables = 0;
        std::vector<uint32_t> mesh_renderables;
        std::vector<Renderable> renderables;
        std::vector<RenderableUniforms> renderables_storage;

        CameraInfo display_camera_info;
        CameraInfo main_camera_info;
        LightInfo light_info;

        bool should_render_atmosphere = false;
        AtmosphereParameters atmosphere_parameters;

        blast::GfxBuffer* renderables_ub = nullptr;

        blast::GfxTexture* skybox_map = nullptr;
    };
}