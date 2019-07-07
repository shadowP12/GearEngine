#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "../Input/Input.h"

static void resizeCallback(GLFWwindow *window, int width, int height);
static void cursorPosCallback(GLFWwindow * window, double xPos, double yPos);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouseButtonCallback(GLFWwindow * window, int button, int action, int mods);
static void mouseScrollCallback(GLFWwindow * window, double xOffset, double yOffset);

class Window
{
public:
	Window(int w = 800, int h = 600);
	~Window();
	void initWindow();
	GLFWwindow* getWindowPtr();
	int getWidth();
	void setWidth(int w);
	int getHeight();
	void setHeight(int h);
	bool getReSize() { return reSize; }
	void setReSize(bool is) { reSize = is; }
	std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);
private:
	int mWidth;
	int mHeight;
	bool reSize;
	GLFWwindow* mWindow;
};

#endif