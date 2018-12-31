#ifndef WINDOW_H
#define WINDOW_H
#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

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
private:
	int mWidth;
	int mHeight;
	GLFWwindow* mWindow;
};
#endif