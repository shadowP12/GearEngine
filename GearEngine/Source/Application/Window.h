#ifndef WINDOW_H
#define WINDOW_H

#include <windows.h>
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vector>
#include <functional>
#include "Input/Input.h"

static void resizeCallback(GLFWwindow *window, int width, int height);
static void cursorPosCallback(GLFWwindow * window, double xPos, double yPos);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouseButtonCallback(GLFWwindow * window, int button, int action, int mods);
static void mouseScrollCallback(GLFWwindow * window, double xOffset, double yOffset);

class Window
{
public:
	Window(int width = 800, int height = 600);
	~Window();
	void* getWindowPtr();
	int getWidth();
	int getHeight();
	void resize(uint32_t width, uint32_t height);
	bool shouldClose();
	void setResizeFunc(std::function<void()> func);
	void update();
private:
	int mWidth;
	int mHeight;
	GLFWwindow* mWindow;
    std::function<void()> mResizeFunc;
};

#endif