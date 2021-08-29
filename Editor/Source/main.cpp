#include "UI.h"
#include "GltfImporter.h"
#include "CameraController.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <GearEngine.h>
#include <Entity/Entity.h>
#include <Entity/Scene.h>
#include <Entity/EntityManager.h>
#include <Entity/Components/CCamera.h>
#include <Entity/Components/CTransform.h>
#include <Entity/Components/CRenderable.h>
#include <Resource/GpuBuffer.h>
#include <Resource/BuiltinResources.h>
#include <Renderer/Renderer.h>
#include <RenderPipeline/RenderPipeline.h>
#include <Input/InputSystem.h>

void CursorPositionCB(GLFWwindow * window, double pos_x, double pos_y) {
    gear::gEngine.GetInputSystem()->OnMousePosition(pos_x, pos_y);
}

void MouseButtonCB(GLFWwindow * window, int button, int action, int mods) {
    gear::gEngine.GetInputSystem()->OnMouseButton(button, action);
}

void MouseScrollCB(GLFWwindow * window, double offset_x, double offset_y) {
    gear::gEngine.GetInputSystem()->OnMouseScroll(offset_y);
}

int main()
{
    gear::gEngine.GetBuiltinResources()->Prepare();

    gear::Entity* main_camera = gear::gEngine.GetEntityManager()->CreateEntity();
    main_camera->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
    main_camera->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(6.0f, 6.0f, 12.0f));
    main_camera->GetComponent<gear::CTransform>()->SetEuler(glm::vec3(-0.358f, 0.46f, 0.0f));
    main_camera->AddComponent<gear::CCamera>()->SetProjection(gear::ProjectionType::PERSPECTIVE, 0.0, 800.0, 600.0, 0.0, 0.1, 1000.0);

    gear::Entity* debug_camera = gear::gEngine.GetEntityManager()->CreateEntity();
    debug_camera->AddComponent<gear::CTransform>()->SetTransform(glm::mat4(1.0f));
    debug_camera->GetComponent<gear::CTransform>()->SetPosition(glm::vec3(0.0f, 0.0f, 12.0f));
    debug_camera->AddComponent<gear::CCamera>()->SetMain(false);
    debug_camera->GetComponent<gear::CCamera>()->SetDisplay(false);
    debug_camera->GetComponent<gear::CCamera>()->SetProjection(gear::ProjectionType::PERSPECTIVE, 0.0, 800.0, 600.0, 0.0, 0.1, 1000.0);

    gear::Scene* editor_scene = gear::gEngine.CreateScene();
    gear::RenderPipeline* editor_pipeline = gear::gEngine.CreateRenderPipeline();
    editor_pipeline->SetScene(editor_scene);
    editor_pipeline->EnableDebug(true);

    GltfAsset* gltf_asset = ImportGltfAsset("./BuiltinResources/GltfFiles/test.gltf");
    for (uint32_t i = 0; i < gltf_asset->entities.size(); ++i) {
        editor_scene->AddEntity(gltf_asset->entities[i]);
    }
    editor_scene->AddEntity(main_camera);
    editor_scene->AddEntity(debug_camera);

    CameraController* camera_controller = new CameraController();
    camera_controller->SetCamera(main_camera);

    // 初始化glfw
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "GearEditor", nullptr, nullptr);
    glfwSetCursorPosCallback(window, CursorPositionCB);
    glfwSetMouseButtonCallback(window, MouseButtonCB);
    glfwSetScrollCallback(window, MouseScrollCB);

    // 初始化ui层
    ImGui::ImGuiInit(window);
    gear::RenderPipeline* ui_pipeline = nullptr;
    ImGui::GetRenderPipeline(&ui_pipeline);

    int window_width, window_height;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glfwGetWindowSize(window, &window_width, &window_height);

        ImGui::BeginUI();
        // 在此插入ui代码
//        bool show_demo_window = true;
//        ImGui::ShowDemoWindow(&show_demo_window);

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

        ImGui::EndUI();

        gear::gEngine.GetRenderer()->BeginFrame(glfwGetWin32Window(window), window_width, window_height);
        editor_pipeline->Exec();
        ui_pipeline->Exec();
        gear::gEngine.GetRenderer()->EndFrame();

        // 重置输入系统状态
        gear::gEngine.GetInputSystem()->Reset();
    }

    // 销毁ui层
    ImGui::ImGuiTerminate();

    // 销毁glfw
    glfwTerminate();

    SAFE_DELETE(camera_controller);

    DestroyGltfAsset(gltf_asset);
    gear::gEngine.DestroyScene(editor_scene);
    gear::gEngine.GetEntityManager()->DestroyEntity(main_camera);
    gear::gEngine.GetEntityManager()->DestroyEntity(debug_camera);
    gear::gEngine.DestroyRenderPipeline(editor_pipeline);
    return 0;
}