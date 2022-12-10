#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include "MaterialCompiler/MaterialCompiler.h"
#include "Resource/BuiltinResources.h"
#include "Input/InputSystem.h"
#include "JobSystem/JobSystem.h"
#include "Animation/AnimationSystem.h"

namespace gear {
    GearEngine::GearEngine() {
        renderer = new Renderer();
        material_compiler = new MaterialCompiler();
        builtin_resources = new BuiltinResources();
        input_system = new InputSystem();
        animation_system = new AnimationSystem();
        job_system = new JobSystem(MAX_THREADS);
    }

    GearEngine::~GearEngine() {
        SAFE_DELETE(animation_system);
        SAFE_DELETE(job_system);
        SAFE_DELETE(input_system);
        SAFE_DELETE(material_compiler);
        SAFE_DELETE(builtin_resources);
        SAFE_DELETE(renderer);
    }

    GearEngine gEngine;
}