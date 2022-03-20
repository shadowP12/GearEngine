#include "ShadowTestScene.h"

#include <GearEngine.h>
#include <Renderer/Renderer.h>

#include <imgui.h>

ShadowTestScene::ShadowTestScene() {
}

ShadowTestScene::~ShadowTestScene() {

}

void ShadowTestScene::Load() {
    scene = new gear::Scene();
    {
        main_camera = gear::gEngine.GetEntityManager()->CreateEntity();
        main_camera->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
        main_camera->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(0.0f, 0.0f, 4.0f));
        //camera->GetComponent<gear::CTransform>()->SetEuler(glm::vec3(-0.358f, 0.46f, 0.0f));
        main_camera->AddComponent<gear::CCamera>()->SetProjection(gear::ProjectionType::PERSPECTIVE, 0.0, 800.0, 600.0, 0.0, 0.1, 100.0);
        //camera->AddComponent<gear::CCamera>()->SetProjection(gear::ProjectionType::ORTHO, -1.0, 1.0, 1.0, -1.0, 0.0, 1.0);
        scene->AddEntity(main_camera);

        debug_camera = gear::gEngine.GetEntityManager()->CreateEntity();
        debug_camera->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
        debug_camera->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(0.0f, 0.0f, 4.0f));
        debug_camera->AddComponent<gear::CCamera>()->SetProjection(gear::ProjectionType::PERSPECTIVE, 0.0, 800.0, 600.0, 0.0, 0.1, 100.0);
        debug_camera->GetComponent<gear::CCamera>()->SetMain(false);
        debug_camera->GetComponent<gear::CCamera>()->SetDisplay(false);
        scene->AddEntity(debug_camera);

        sun = gear::gEngine.GetEntityManager()->CreateEntity();
        sun->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
        sun->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(0.0f, 10.0f, 0.0f));
        sun->GetComponent<gear::CTransform>()->SetEuler(glm::vec3(glm::radians(120.0f), 0.0f, 0.0f));
        sun->AddComponent<gear::CLight>();
        scene->AddEntity(sun);

        camera_controller = new CameraController();
        camera_controller->SetCamera(main_camera);

        gltf_asset = ImportGltfAsset("./BuiltinResources/GltfFiles/test.gltf");

        for (uint32_t i = 0; i < gltf_asset->entities.size(); ++i) {
            if (gltf_asset->entities[i]->HasComponent<gear::CMesh>()) {
                gltf_asset->entities[i]->GetComponent<gear::CMesh>()->SetCastShadow(true);
                gltf_asset->entities[i]->GetComponent<gear::CMesh>()->SetReceiveShadow(true);
                scene->AddEntity(gltf_asset->entities[i]);
            }
        }

        // 加载天空盒以及IBL资源
        gear::Texture* equirectangular_map = ImportTexture2DWithFloat("./BuiltinResources/Textures/Ridgecrest_Road_Ref.hdr");
        skybox_map = gear::gEngine.GetRenderer()->EquirectangularMapToCubemap(equirectangular_map, 512);
        irradiance_map = gear::gEngine.GetRenderer()->ComputeIrradianceMap(skybox_map);
        prefiltered_map = gear::gEngine.GetRenderer()->ComputePrefilteredMap(skybox_map);
        brdf_lut = gear::gEngine.GetRenderer()->ComputeBRDFLut();
        SAFE_DELETE(equirectangular_map);

        sun->AddComponent<gear::CSkybox>()->SetCubeMap(skybox_map);
        ibl = gear::gEngine.GetEntityManager()->CreateEntity();
        ibl->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
        ibl->AddComponent<gear::CLight>();
        ibl->GetComponent<gear::CLight>()->SetLightType(gear::CLight::LightType::IBL);
        ibl->GetComponent<gear::CLight>()->SetIrradianceMap(irradiance_map);
        ibl->GetComponent<gear::CLight>()->SetPrefilteredMap(prefiltered_map);
        ibl->GetComponent<gear::CLight>()->SetBRDFLut(brdf_lut);
        scene->AddEntity(ibl);
    }
}

void ShadowTestScene::Clear() {
    DestroyGltfAsset(gltf_asset);
    gear::gEngine.GetEntityManager()->DestroyEntity(sun);
    gear::gEngine.GetEntityManager()->DestroyEntity(ibl);
    gear::gEngine.GetEntityManager()->DestroyEntity(main_camera);
    gear::gEngine.GetEntityManager()->DestroyEntity(debug_camera);
    SAFE_DELETE(skybox_map);
    SAFE_DELETE(irradiance_map);
    SAFE_DELETE(prefiltered_map);
    SAFE_DELETE(brdf_lut);
    SAFE_DELETE(camera_controller);
    SAFE_DELETE(scene);
}

void ShadowTestScene::DrawUI() {
    ImGui::Begin("Change Camera");
    if (ImGui::Button("Main Camera")) {
        main_camera->GetComponent<gear::CCamera>()->SetDisplay(true);
        debug_camera->GetComponent<gear::CCamera>()->SetDisplay(false);
        camera_controller->SetCamera(main_camera);
    }
    if (ImGui::Button("Debug Camera")) {
        main_camera->GetComponent<gear::CCamera>()->SetDisplay(false);
        debug_camera->GetComponent<gear::CCamera>()->SetDisplay(true);
        camera_controller->SetCamera(debug_camera);
    }
    ImGui::End();
}

gear::Scene * ShadowTestScene::GetScene() {
    return scene;
}

