#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <Engine/Renderer/Renderer.h>
#include <Engine/GearEngine.h>

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
    gear::gEngine.getRenderer()->initSurface(glfwGetWin32Window(gWindowPtr));
    gear::gEngine.getRenderer()->setSurfaceSizeFunc(getSurfaceSize);
    gear::gEngine.getRenderer()->resize(800, 600);
    while (!glfwWindowShouldClose(gWindowPtr)) {
        gear::gEngine.getRenderer()->render();
        glfwPollEvents();
    }
    glfwDestroyWindow(gWindowPtr);
    glfwTerminate();
    return 0;
}