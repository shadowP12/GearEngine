#include "UI.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <GearEngine.h>
#include <Entity/Entity.h>
#include <Entity/Scene.h>
#include <Entity/EntityManager.h>
#include <Resource/GpuBuffer.h>
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
    gear::Scene* scene = gear::gEngine.CreateScene();
    gear::RenderPipeline* render_pipeline = gear::gEngine.CreateRenderPipeline();
    render_pipeline->SetScene(scene);

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
        bool show_demo_window = true;
        ImGui::ShowDemoWindow(&show_demo_window);
        ImGui::EndUI();

        gear::gEngine.GetRenderer()->BeginFrame(glfwGetWin32Window(window), window_width, window_height);
        ui_pipeline->Exec();
        gear::gEngine.GetRenderer()->EndFrame();

        // 重置输入系统状态
        gear::gEngine.GetInputSystem()->Reset();
    }

    // 销毁ui层
    ImGui::ImGuiTerminate();

    // 销毁glfw
    glfwTerminate();

    gear::gEngine.DestroyScene(scene);
    gear::gEngine.DestroyRenderPipeline(render_pipeline);
    return 0;
}