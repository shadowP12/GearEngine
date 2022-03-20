#pragma once

#include "TestScene.h"
#include "../GltfImporter.h"
#include "../TextureImporter.h"
#include "../CameraController.h"

#include <Entity/Scene.h>
#include <Entity/Entity.h>
#include <Entity/EntityManager.h>
#include <Entity/Components/CLight.h>
#include <Entity/Components/CCamera.h>
#include <Entity/Components/CTransform.h>
#include <Entity/Components/CMesh.h>
#include <Entity/Components/CSkybox.h>
#include <Entity/Components/CAnimation.h>
#include <Resource/GpuBuffer.h>
#include <Resource/Texture.h>
#include <Resource/Material.h>
#include <Resource/BuiltinResources.h>
#include <MaterialCompiler/MaterialCompiler.h>
#include <Animation/Skeleton.h>
#include <Animation/AnimationClip.h>
#include <Animation/AnimationInstance.h>

class AnimationTestScene : public TestScene {
public:
    AnimationTestScene();

    ~AnimationTestScene();

    void Load() override;

    void Clear() override;

    void DrawUI() override;

    gear::Scene* GetScene() override;

protected:
    gear::Scene* scene = nullptr;
    gear::Entity* camera = nullptr;
    gear::Entity* sun = nullptr;
    gear::Entity* ibl = nullptr;
    gear::Texture* skybox_map = nullptr;
    gear::Texture* irradiance_map = nullptr;
    gear::Texture* prefiltered_map = nullptr;
    gear::Texture* brdf_lut = nullptr;
    GltfAsset* gltf_asset = nullptr;
    gear::SimpleAnimationInstance* animation_instance = nullptr;
    CameraController* camera_controller = nullptr;
};