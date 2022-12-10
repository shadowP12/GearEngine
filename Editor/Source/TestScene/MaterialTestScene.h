#pragma once

#include "TestScene.h"
#include "../GltfImporter.h"
#include "../TextureImporter.h"
#include "../CameraController.h"
#include <Entity/Scene.h>
#include <Entity/Entity.h>
#include <Entity/Components/CLight.h>
#include <Entity/Components/CCamera.h>
#include <Entity/Components/CTransform.h>
#include <Entity/Components/CMesh.h>
#include <Entity/Components/CSkybox.h>
#include <Entity/Components/CAnimation.h>
#include <Resource/Texture.h>
#include <Resource/Material.h>
#include <Resource/BuiltinResources.h>
#include <MaterialCompiler/MaterialCompiler.h>

class MaterialTestScene : public TestScene {
public:
    MaterialTestScene();

    virtual ~MaterialTestScene();

    void Load() override;

    void Clear() override;

    void DrawUI() override;

    std::shared_ptr<gear::Scene> GetScene() override;

protected:
    std::shared_ptr<gear::Scene> scene = nullptr;
    std::shared_ptr<gear::Entity> main_camera = nullptr;
    std::shared_ptr<gear::Entity> sun = nullptr;
    std::shared_ptr<gear::Entity> ibl = nullptr;
    std::shared_ptr<blast::GfxTexture> skybox_map = nullptr;
    std::shared_ptr<blast::GfxTexture> irradiance_map = nullptr;
    std::shared_ptr<blast::GfxTexture> prefiltered_map = nullptr;
    std::shared_ptr<blast::GfxTexture> brdf_lut = nullptr;
    GltfAsset* gltf_asset = nullptr;
    CameraController* camera_controller = nullptr;
};
