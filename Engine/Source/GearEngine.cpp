#include "GearEngine.h"
#include "Entity/EntityManager.h"
#include "Renderer/Renderer.h"
#include "MaterialCompiler/MaterialCompiler.h"
#include "Resource/BuiltinResources.h"
#include "Input/InputSystem.h"
#include "JobSystem/JobSystem.h"
#include "Animation/AnimationSystem.h"

#include <Blast/Gfx/GfxDevice.h>
#include <Blast/Gfx/Vulkan/VulkanDevice.h>
#include <Blast/Utility/ShaderCompiler.h>
#include <Blast/Utility/VulkanShaderCompiler.h>

namespace gear {
    GearEngine::GearEngine() {
        device = new blast::VulkanDevice();
        shader_compiler = new blast::VulkanShaderCompiler();
        renderer = new Renderer();
        material_compiler = new MaterialCompiler();
        builtin_resources = new BuiltinResources();
        entity_manager = new EntityManager();
        input_system = new InputSystem();
        animation_system = new AnimationSystem();
        job_system = new JobSystem(MAX_THREADS);
    }

    GearEngine::~GearEngine() {
        SAFE_DELETE(animation_system);
        SAFE_DELETE(job_system);
        SAFE_DELETE(input_system);
        SAFE_DELETE(shader_compiler);
        SAFE_DELETE(renderer);
        SAFE_DELETE(material_compiler);
        SAFE_DELETE(builtin_resources);
        SAFE_DELETE(entity_manager);
        SAFE_DELETE(device);
    }

    GearEngine gEngine;
}