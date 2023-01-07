#include "TransparencyTestScene.h"
#include <GearEngine.h>
#include <Renderer/Renderer.h>
#include <Resource/Material.h>
#include <imgui.h>
#include "../EditorMisc.h"

TransparencyTestScene::TransparencyTestScene() {
}

TransparencyTestScene::~TransparencyTestScene() {
}

void TransparencyTestScene::Load() {
    scene = gear::Scene::Create("TransparencyTestScene");
    main_camera = gear::Entity::Create("MainCamera");
    main_camera->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
    main_camera->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(0.0f, 0.0f, 4.0f));
    main_camera->AddComponent<gear::CCamera>()->SetProjection(gear::ProjectionType::PERSPECTIVE, 0.0, 800.0, 600.0, 0.0, 0.1, 100.0);
    scene->AddEntity(main_camera);

    sun = gear::Entity::Create("Sun");
    sun->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
    sun->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(0.0f, 10.0f, 0.0f));
    sun->GetComponent<gear::CTransform>()->SetEuler(glm::vec3(glm::radians(120.0f), 0.0f, 0.0f));
    sun->AddComponent<gear::CLight>();
    sun->GetComponent<gear::CLight>()->SetColor(glm::vec3(0.8f, 0.8f, 0.9f));
    sun->GetComponent<gear::CLight>()->SetIntensity(120000.0f);
    scene->AddEntity(sun);

    camera_controller = new CameraController();
    camera_controller->SetCamera(main_camera.get());

    transparency_test_ma = gear::gEngine.GetMaterialCompiler()->Compile(EditorMisc::GetEditorResourcesDir() + "/Materials/transparency_test.mat", true);
    gltf_asset = ImportGltfAsset(EditorMisc::GetEditorResourcesDir() + "/GltfFiles/transparency_test/transparency_test.gltf");
    for (auto iter : gltf_asset->entities) {
        auto entity = iter.second;
        if (entity->HasComponent<gear::CMesh>()) {
            std::shared_ptr<gear::MaterialInstance> transparency_test_mi = transparency_test_ma->CreateInstance();
            entity->GetComponent<gear::CMesh>()->SetMaterial(0, transparency_test_mi);
            entity->GetComponent<gear::CMesh>()->SetCastShadow(true);
            entity->GetComponent<gear::CMesh>()->SetReceiveShadow(false);
            scene->AddEntity(entity);
            transparency_test_mis.push_back(transparency_test_mi);
        }
    }

    // Skybox and ibl
    std::shared_ptr<blast::GfxTexture> equirectangular_map = ImportTexture2DWithFloat(EditorMisc::GetEditorResourcesDir() + "/Textures/Ridgecrest_Road_Ref.hdr");
    skybox_map = gear::gEngine.GetRenderer()->EquirectangularMapToCubemap(equirectangular_map, 512);
    irradiance_map = gear::gEngine.GetRenderer()->ComputeIrradianceMap(skybox_map);
    prefiltered_map = gear::gEngine.GetRenderer()->ComputePrefilteredMap(skybox_map);
    brdf_lut = gear::gEngine.GetRenderer()->ComputeBRDFLut();

    sun->AddComponent<gear::CSkybox>()->SetCubeMap(skybox_map);
    ibl = gear::Entity::Create("IBL");
    ibl->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
    ibl->AddComponent<gear::CLight>();
    ibl->GetComponent<gear::CLight>()->SetLightType(gear::CLight::LightType::IBL);
    ibl->GetComponent<gear::CLight>()->SetIrradianceMap(irradiance_map);
    ibl->GetComponent<gear::CLight>()->SetPrefilteredMap(prefiltered_map);
    ibl->GetComponent<gear::CLight>()->SetBRDFLut(brdf_lut);
    scene->AddEntity(ibl);
}

void TransparencyTestScene::Clear() {
    SAFE_DELETE(camera_controller);
    DestroyGltfAsset(gltf_asset);
}

void TransparencyTestScene::DrawUI() {
    // Impl
}

std::shared_ptr<gear::Scene> TransparencyTestScene::GetScene() {
    return scene;
}