#include "SkyAtmosphereTestScene.h"

#include <GearEngine.h>
#include <Renderer/Renderer.h>
#include <Resource/Material.h>
#include <Entity/Components/CAtmosphere.h>

#include <imgui.h>

SkyAtmosphereTestScene::SkyAtmosphereTestScene() {
}

SkyAtmosphereTestScene::~SkyAtmosphereTestScene() {
}

void SkyAtmosphereTestScene::Load() {
    scene = new gear::Scene();
    {
        main_camera = gear::gEngine.GetEntityManager()->CreateEntity();
        main_camera->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
        main_camera->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(0.0f, 1.0f, 0.0f));
        main_camera->AddComponent<gear::CCamera>()->SetProjection(gear::ProjectionType::PERSPECTIVE, 0.0, 800.0, 600.0, 0.0, 0.1, 100.0);
        scene->AddEntity(main_camera);

        sun = gear::gEngine.GetEntityManager()->CreateEntity();
        sun->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
        sun->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(0.0f, 10000.0f, 0.0f));
        sun->GetComponent<gear::CTransform>()->SetEuler(glm::vec3(glm::radians(120.0f), 0.0f, 0.0f));
        sun->AddComponent<gear::CLight>();
        sun->GetComponent<gear::CLight>()->SetColor(glm::vec3(0.8f, 0.8f, 0.9f));
        sun->GetComponent<gear::CLight>()->SetIntensity(120000.0f);
		// 大气组件
		sun->AddComponent<gear::CAtmosphere>();
		sun->GetComponent<gear::CAtmosphere>()->ShouldRenderAtmosphere(true);
        scene->AddEntity(sun);

        camera_controller = new CameraController();
        camera_controller->SetCamera(main_camera);

        transparency_test_ma = gear::gEngine.GetMaterialCompiler()->Compile("../BuiltinResources/Materials/transparency_test.mat", true);
        gltf_asset = ImportGltfAsset("../BuiltinResources/GltfFiles/transparency_test/transparency_test.gltf");
        for (uint32_t i = 0; i < gltf_asset->entities.size(); ++i) {
            if (gltf_asset->entities[i]->HasComponent<gear::CMesh>()) {
                gear::MaterialInstance* transparency_test_mi = transparency_test_ma->CreateInstance();
                gltf_asset->entities[i]->GetComponent<gear::CMesh>()->GetSubMeshs()[0].mi = transparency_test_mi;
                gltf_asset->entities[i]->GetComponent<gear::CMesh>()->SetCastShadow(true);
                gltf_asset->entities[i]->GetComponent<gear::CMesh>()->SetReceiveShadow(false);
                scene->AddEntity(gltf_asset->entities[i]);
                transparency_test_mis.push_back(transparency_test_mi);
            }
        }

        // 加载天空盒以及IBL资源
        gear::Texture* equirectangular_map = ImportTexture2DWithFloat("../BuiltinResources/Textures/Ridgecrest_Road_Ref.hdr");
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

void SkyAtmosphereTestScene::Clear() {
    for (int i = 0; i < transparency_test_mis.size(); ++i) {
        SAFE_DELETE(transparency_test_mis[i]);
    }
    transparency_test_mis.clear();
    SAFE_DELETE(transparency_test_ma);
    DestroyGltfAsset(gltf_asset);
    gear::gEngine.GetEntityManager()->DestroyEntity(sun);
    gear::gEngine.GetEntityManager()->DestroyEntity(ibl);
    gear::gEngine.GetEntityManager()->DestroyEntity(main_camera);
    SAFE_DELETE(skybox_map);
    SAFE_DELETE(irradiance_map);
    SAFE_DELETE(prefiltered_map);
    SAFE_DELETE(brdf_lut);
    SAFE_DELETE(camera_controller);
    SAFE_DELETE(scene);
}

void SkyAtmosphereTestScene::DrawUI() {
	ImGui::Begin("Sun Setting");
	glm::vec3 e = glm::degrees(sun->GetComponent<gear::CTransform>()->GetEuler());
	ImGui::DragFloat("pitch", &e.x, 1.0f, 0.0f, 360.0f);
	ImGui::DragFloat("yaw", &e.y, 1.0f, 0.0f, 360.0f);
	sun->GetComponent<gear::CTransform>()->SetEuler(glm::radians(e));
	ImGui::End();
}

gear::Scene* SkyAtmosphereTestScene::GetScene() {
    return scene;
}