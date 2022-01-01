#pragma once
#include "Core/GearDefine.h"


namespace blast {
    class GfxDevice;
    class ShaderCompiler;
}

namespace gear {
    class Renderer;
    class MaterialCompiler;
    class BuiltinResources;
    class EntityManager;
    class InputSystem;
    class JobSystem;
    class GearEngine {
    public:
        GearEngine();

        ~GearEngine();

        blast::GfxDevice* GetDevice() { return device;}

        blast::ShaderCompiler* GetShaderCompiler() { return shader_compiler; }

        Renderer* GetRenderer() { return renderer; }

        MaterialCompiler* GetMaterialCompiler() { return material_compiler;}

        BuiltinResources* GetBuiltinResources() { return builtin_resources; }

        EntityManager* GetEntityManager() { return entity_manager; }

        InputSystem* GetInputSystem() { return input_system; }

        JobSystem* GetJobSystem() { return job_system; }

    private:
        blast::GfxDevice* device = nullptr;
        blast::ShaderCompiler* shader_compiler = nullptr;
        Renderer* renderer = nullptr;
        MaterialCompiler* material_compiler = nullptr;
        BuiltinResources* builtin_resources = nullptr;
        EntityManager* entity_manager = nullptr;
        InputSystem* input_system = nullptr;
        JobSystem* job_system = nullptr;
    };

    extern GearEngine gEngine;
}