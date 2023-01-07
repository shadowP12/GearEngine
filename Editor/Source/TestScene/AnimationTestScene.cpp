#include "AnimationTestScene.h"
#include <GearEngine.h>
#include <Renderer/Renderer.h>
#include <imgui.h>
#include "../EditorMisc.h"

AnimationTestScene::AnimationTestScene() {
}

AnimationTestScene::~AnimationTestScene() {
}

void AnimationTestScene::Load() {
    scene = gear::Scene::Create("AnimationTestScene");
    camera = gear::Entity::Create("Camera");
    camera->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
    camera->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(0.0f, 4.5f, 12.0f));
    //camera->GetComponent<gear::CTransform>()->SetEuler(glm::vec3(-0.358f, 0.46f, 0.0f));
    camera->AddComponent<gear::CCamera>()->SetProjection(gear::ProjectionType::PERSPECTIVE, 0.0, 800.0, 600.0, 0.0, 0.1, 100.0);
    //camera->AddComponent<gear::CCamera>()->SetProjection(gear::ProjectionType::ORTHO, -1.0, 1.0, 1.0, -1.0, 0.0, 1.0);
    scene->AddEntity(camera);

    sun = gear::Entity::Create("Sun");
    sun->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
    sun->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(0.0f, 10.0f, 0.0f));
    sun->GetComponent<gear::CTransform>()->SetEuler(glm::vec3(glm::radians(120.0f), 0.0f, 0.0f));
    sun->AddComponent<gear::CLight>();
    sun->GetComponent<gear::CLight>()->SetColor(glm::vec3(0.8f, 0.8f, 0.9f));
    sun->GetComponent<gear::CLight>()->SetIntensity(120000.0f);
    scene->AddEntity(sun);

    camera_controller = new CameraController();
    camera_controller->SetCamera(camera.get());

    gltf_asset = ImportGltfAsset(EditorMisc::GetEditorResourcesDir() + "/GltfFiles/mech_drone/scene.gltf");
    std::shared_ptr<gear::Skeleton> skeleton = nullptr;
    std::shared_ptr<gear::AnimationClip> animation_clip = nullptr;
    for (auto iter : gltf_asset->skeletons) {
        skeleton = iter.second;
        break;
    }
    for (auto iter : gltf_asset->animation_clips) {
        animation_clip = iter.second;
        break;
    }

    animation_instance = std::make_shared<gear::SimpleAnimationInstance>();
    animation_instance->SetAnimationMode(gear::AnimationMode::LOOP);
    animation_instance->SetSkeleton(skeleton);
    animation_instance->SetAnimationClip(animation_clip);
    // Debug
    //animation_instance->Puase();

    uint32_t idx = 0;
    for (auto iter : gltf_asset->entities) {
        auto entity = iter.second;
        //RootNode
        if (entity->GetName() == "node_4") {
            entity->GetComponent<gear::CTransform>()->SetEuler(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
            entity->GetComponent<gear::CTransform>()->SetScale(glm::vec3(0.03f));
        }

        if (entity->HasComponent<gear::CMesh>()) {
            entity->GetComponent<gear::CMesh>()->SetCastShadow(true);
            entity->GetComponent<gear::CMesh>()->SetReceiveShadow(true);

            if (entity->HasComponent<gear::CMesh>() && entity->GetComponent<gear::CMesh>()->GetSkeleton()) {
                // Modify bbox
                gear::BBox new_bbox = entity->GetComponent<gear::CMesh>()->GetBoundBox();
                float d = glm::distance(new_bbox.bb_max, new_bbox.bb_min);
                new_bbox.bb_max.y += d / 4.0f;
                new_bbox.bb_min.y -= d / 4.0f;
                entity->GetComponent<gear::CMesh>()->SetBoundBox(new_bbox);

                // Created animation component
                entity->AddComponent<gear::CAnimation>();
                entity->GetComponent<gear::CAnimation>()->SetAnimationInstance(animation_instance);
                entity->GetComponent<gear::CAnimation>()->Play();

                scene->AddEntity(entity);
            }
        }
        idx++;
    }

    // Skybox and ibl
    std::shared_ptr<blast::GfxTexture> equirectangular_map = ImportTexture2DWithFloat("../BuiltinResources/Textures/Ridgecrest_Road_Ref.hdr");
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

void AnimationTestScene::Clear() {
    DestroyGltfAsset(gltf_asset);
    SAFE_DELETE(camera_controller);
}

void AnimationTestScene::DrawUI() {
    if (animation_instance) {
        ImGui::Begin("Anim Debug");
        float time = animation_instance->GetTime();
        ImGui::DragFloat("time", &time, 0.001f, 0.0f, 10000.0f);
        animation_instance->SetTime(time);
        ImGui::End();
    }
}

std::shared_ptr<gear::Scene> AnimationTestScene::GetScene() {
    return scene;
}

