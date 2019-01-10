#ifndef WINDOW_H
#define WINDOW_H
#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>
#include "../Input/Input.h"

void resizeCallback(GLFWwindow *window, int width, int height);
void cursorPosCallback(GLFWwindow * window, double xPos, double yPos);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseButtonCallback(GLFWwindow * window, int button, int action, int mods);
void mouseScrollCallback(GLFWwindow * window, double xOffset, double yOffset);

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
	void setReSize(bool is) { reSize = is; }
private:
	int mWidth;
	int mHeight;
	bool reSize;
	GLFWwindow* mWindow;
};

#endif