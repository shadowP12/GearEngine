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

class MaterialTestScene : public TestScene {
public:
    MaterialTestScene();

    ~MaterialTestScene();

    void Load() override;

    void Clear() override;

    void DrawUI() override;

    gear::Scene* GetScene() override;

protected:
    gear::Scene* scene = nullptr;
    gear::Entity* main_camera = nullptr;
    gear::Entity* sun = nullptr;
    gear::Entity* ibl = nullptr;
    gear::Texture* skybox_map = nullptr;
    gear::Texture* irradiance_map = nullptr;
    gear::Texture* prefiltered_map = nullptr;
    gear::Texture* brdf_lut = nullptr;
    GltfAsset* gltf_asset = nullptr;
    CameraController* camera_controller = nullptr;
};
