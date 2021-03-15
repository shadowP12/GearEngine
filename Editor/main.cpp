#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <Engine/Renderer/Renderer.h>

GLFWwindow* gWindowPtr = nullptr;

void getSurfaceSize(int* w, int* h) {
    glfwGetFramebufferSize(gWindowPtr, w, h);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    gWindowPtr = glfwCreateWindow(800, 600, "GearEditor", nullptr, nullptr);
    glfwSetWindowUserPointer(gWindowPtr, nullptr);
    gear::Renderer::startUp();
    gear::Renderer::instance().initSurface(glfwGetWin32Window(gWindowPtr));
    gear::Renderer::instance().setSurfaceSizeFunc(getSurfaceSize);
    gear::Renderer::instance().resize(800, 600);
    while (!glfwWindowShouldClose(gWindowPtr)) {
        gear::Renderer::instance().render();
        glfwPollEvents();
    }
    glfwDestroyWindow(gWindowPtr);
    glfwTerminate();

    gear::Renderer::shutDown();
    return 0;
}