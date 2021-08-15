#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <GearEngine.h>
#include <Entity/Entity.h>
#include <Entity/Scene.h>
#include <Entity/EntityManager.h>
#include <Renderer/Renderer.h>
#include <RenderPipeline/RenderPipeline.h>

struct Vertex {
    float pos[3];
    float uv[2];
};

float vertices[] = {
        -0.5f,  0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 1.0f
};

unsigned int indices[] = {
        0, 1, 2, 2, 3, 0
};

int main()
{
    gear::Scene* scene = gear::gEngine.CreateScene();
    gear::RenderPipeline* render_pipeline = gear::gEngine.CreateRenderPipeline();
    render_pipeline->SetScene(scene);

    // 初始化glfw
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "GearEditor", nullptr, nullptr);

    int window_width, window_height;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glfwGetWindowSize(window, &window_width, &window_height);

        gear::gEngine.GetRenderer()->BeginFrame(glfwGetWin32Window(window), window_width, window_height);
        render_pipeline->Exec();
        gear::gEngine.GetRenderer()->EndFrame();
    }

    // 销毁glfw
    glfwTerminate();

    gear::gEngine.DestroyScene(scene);
    gear::gEngine.DestroyRenderPipeline(render_pipeline);
    return 0;
}