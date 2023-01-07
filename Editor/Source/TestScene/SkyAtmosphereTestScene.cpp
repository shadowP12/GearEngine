#include "SkyAtmosphereTestScene.h"
#include <GearEngine.h>
#include <Renderer/Renderer.h>
#include <Resource/Material.h>
#include <Entity/Components/CAtmosphere.h>
#include <imgui.h>
#include "../EditorMisc.h"

SkyAtmosphereTestScene::SkyAtmosphereTestScene() {
}

SkyAtmosphereTestScene::~SkyAtmosphereTestScene() {
}

void SkyAtmosphereTestScene::Load() {
    scene = gear::Scene::Create("SkyAtmosphereTestScene");
    main_camera = gear::Entity::Create("MainCamera");
    main_camera->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
    main_camera->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(0.0f, 1.0f, 0.0f));
    main_camera->AddComponent<gear::CCamera>()->SetProjection(gear::ProjectionType::PERSPECTIVE, 0.0, 800.0, 600.0, 0.0, 0.1, 100.0);
    scene->AddEntity(main_camera);

    sun = gear::Entity::Create("Sun");
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
    camera_controller->SetCamera(main_camera.get());

    gltf_asset = ImportGltfAsset(EditorMisc::GetEditorResourcesDir() + "/GltfFiles/test.gltf");
    for (auto iter : gltf_asset->entities) {
        auto entity = iter.second;
        if (entity->HasComponent<gear::CMesh>()) {
            entity->GetComponent<gear::CMesh>()->SetCastShadow(true);
            entity->GetComponent<gear::CMesh>()->SetReceiveShadow(false);
            scene->AddEntity(entity);
        }
    }
}

void SkyAtmosphereTestScene::Clear() {
    DestroyGltfAsset(gltf_asset);
    SAFE_DELETE(camera_controller);
}

void SkyAtmosphereTestScene::DrawUI() {
    ImGui::Begin("Sun Setting");
    glm::vec3 e = glm::degrees(sun->GetComponent<gear::CTransform>()->GetEuler());
    ImGui::DragFloat("pitch", &e.x, 1.0f, 0.0f, 360.0f);
    ImGui::DragFloat("yaw", &e.y, 1.0f, 0.0f, 360.0f);
    sun->GetComponent<gear::CTransform>()->SetEuler(glm::radians(e));
    ImGui::End();
}

std::shared_ptr<gear::Scene> SkyAtmosphereTestScene::GetScene() {
    return scene;
}