#pragma once
#include "Core/GearDefine.h"

namespace gear {
    class Renderer;
    class MaterialCompiler;
    class BuiltinResources;
    class InputSystem;
    class JobSystem;
    class AnimationSystem;
    class GearEngine {
    public:
        GearEngine();

        ~GearEngine();

        Renderer* GetRenderer() { return renderer; }

        MaterialCompiler* GetMaterialCompiler() { return material_compiler;}

        BuiltinResources* GetBuiltinResources() { return builtin_resources; }

        InputSystem* GetInputSystem() { return input_system; }

        JobSystem* GetJobSystem() { return job_system; }

        AnimationSystem* GetAnimationSystem() { return animation_system; }

    private:
        Renderer* renderer = nullptr;
        MaterialCompiler* material_compiler = nullptr;
        BuiltinResources* builtin_resources = nullptr;
        InputSystem* input_system = nullptr;
        JobSystem* job_system = nullptr;
        AnimationSystem* animation_system = nullptr;
    };

    extern GearEngine gEngine;
}