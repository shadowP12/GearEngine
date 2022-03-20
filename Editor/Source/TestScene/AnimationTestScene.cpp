#include "AnimationTestScene.h"

#include <GearEngine.h>
#include <Renderer/Renderer.h>

#include <imgui.h>

AnimationTestScene::AnimationTestScene() {
}

AnimationTestScene::~AnimationTestScene() {

}

void AnimationTestScene::Load() {
    scene = new gear::Scene();
    {
        camera = gear::gEngine.GetEntityManager()->CreateEntity();
        camera->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
        camera->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(0.0f, 4.5f, 12.0f));
        //camera->GetComponent<gear::CTransform>()->SetEuler(glm::vec3(-0.358f, 0.46f, 0.0f));
        camera->AddComponent<gear::CCamera>()->SetProjection(gear::ProjectionType::PERSPECTIVE, 0.0, 800.0, 600.0, 0.0, 0.1, 100.0);
        //camera->AddComponent<gear::CCamera>()->SetProjection(gear::ProjectionType::ORTHO, -1.0, 1.0, 1.0, -1.0, 0.0, 1.0);
        scene->AddEntity(camera);

        sun = gear::gEngine.GetEntityManager()->CreateEntity();
        sun->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
        sun->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(0.0f, 10.0f, 0.0f));
        sun->GetComponent<gear::CTransform>()->SetEuler(glm::vec3(glm::radians(120.0f), 0.0f, 0.0f));
        sun->AddComponent<gear::CLight>();
        scene->AddEntity(sun);

        camera_controller = new CameraController();
        camera_controller->SetCamera(camera);

        gltf_asset = ImportGltfAsset("./BuiltinResources/GltfFiles/mech_drone/scene.gltf");

        animation_instance = new gear::SimpleAnimationInstance();
        animation_instance->SetAnimationMode(gear::AnimationMode::LOOP);
        animation_instance->SetSkeleton(gltf_asset->skeletons[0]);
        animation_instance->SetAnimationClip(gltf_asset->animation_clips[0]);
        // debug
        //animation_instance->Puase();

        for (uint32_t i = 0; i < gltf_asset->entities.size(); ++i) {
            if (i == 3) {
                gltf_asset->entities[i]->GetComponent<gear::CTransform>()->SetEuler(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
                gltf_asset->entities[i]->GetComponent<gear::CTransform>()->SetScale(glm::vec3(0.03f));
            }

            if (gltf_asset->entities[i]->HasComponent<gear::CMesh>()) {
                gltf_asset->entities[i]->GetComponent<gear::CMesh>()->SetCastShadow(true);
                gltf_asset->entities[i]->GetComponent<gear::CMesh>()->SetReceiveShadow(true);

                gear::VertexBuffer* vb = gltf_asset->entities[i]->GetComponent<gear::CMesh>()->GetSubMeshs()[0].vb;
                if (vb->GetVertexLayoutType() == gear::VLT_SKIN_MESH) {
                    // 绑定skeleton
                    gltf_asset->entities[i]->GetComponent<gear::CMesh>()->SetSkeleton(gltf_asset->skeletons[0]);

                    // 创建动画组件
                    gltf_asset->entities[i]->AddComponent<gear::CAnimation>();
                    gltf_asset->entities[i]->GetComponent<gear::CAnimation>()->SetAnimationInstance(animation_instance);
                    gltf_asset->entities[i]->GetComponent<gear::CAnimation>()->Play();

                    scene->AddEntity(gltf_asset->entities[i]);
                }
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

void AnimationTestScene::Clear() {
    DestroyGltfAsset(gltf_asset);
    gear::gEngine.GetEntityManager()->DestroyEntity(sun);
    gear::gEngine.GetEntityManager()->DestroyEntity(ibl);
    gear::gEngine.GetEntityManager()->DestroyEntity(camera);
    SAFE_DELETE(animation_instance);
    SAFE_DELETE(skybox_map);
    SAFE_DELETE(irradiance_map);
    SAFE_DELETE(prefiltered_map);
    SAFE_DELETE(brdf_lut);
    SAFE_DELETE(camera_controller);
    SAFE_DELETE(scene);
}

void AnimationTestScene::DrawUI() {
//    bool show_demo_window = true;
//    ImGui::ShowDemoWindow(&show_demo_window);

    if (animation_instance) {
        ImGui::Begin("Anim Debug");
        float time = animation_instance->GetTime();
        ImGui::DragFloat("time", &time, 0.001f, 0.0f, 10000.0f);
        animation_instance->SetTime(time);
        ImGui::End();
    }
}

gear::Scene * AnimationTestScene::GetScene() {
    return scene;
}

