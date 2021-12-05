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

    private:
        blast::GfxDevice* device = nullptr;
        blast::ShaderCompiler* shader_compiler = nullptr;
        Renderer* renderer = nullptr;
        MaterialCompiler* material_compiler = nullptr;
        BuiltinResources* builtin_resources = nullptr;
        EntityManager* entity_manager = nullptr;
    };

    extern GearEngine gEngine;
}