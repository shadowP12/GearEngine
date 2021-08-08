#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <GearEngine.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderScene.h>

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
    // 初始化glfw
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "GearEditor", nullptr, nullptr);

    // 传入窗口句柄
    gear::gEngine.getRenderer()->initSurface(glfwGetWin32Window(window));

    int window_width, window_height;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glfwGetWindowSize(window, &window_width, &window_height);

        gear::gEngine.getRenderer()->beginFrame(window_width, window_height);
        gear::gEngine.getRenderer()->endFrame();
    }

    // 确保渲染器结束所有的工作
    gear::gEngine.getRenderer()->terminate();

    // 销毁glfw
    glfwTerminate();
    return 0;
}