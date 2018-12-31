#include "Window.h"

Window::Window(int w, int h)
{
	mWidth = w;
	mHeight = h;
}

Window::~Window()
{
}

void Window::initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	mWindow = glfwCreateWindow(mWidth, mHeight, "Gear", nullptr, nullptr);
	glfwSetWindowUserPointer(mWindow, this);
}

GLFWwindow * Window::getWindowPtr()
{
	return mWindow;
}

int Window::getWidth()
{
	return mWidth;
}

void Window::setWidth(int w)
{
	mWidth = w;
}

int Window::getHeight()
{
	return mHeight;
}

void Window::setHeight(int h)
{
	mHeight = h;
}
