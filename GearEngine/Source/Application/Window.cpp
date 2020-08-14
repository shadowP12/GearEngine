#include "Window.h"

Window::Window(int width, int height)
	:mWidth(width),mHeight(height)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	mWindow = glfwCreateWindow(mWidth, mHeight, "Gear", nullptr, nullptr);
	glfwSetWindowUserPointer(mWindow, this);
	glfwSetFramebufferSizeCallback(mWindow, resizeCallback);
	glfwSetKeyCallback(mWindow, keyCallback);
	glfwSetMouseButtonCallback(mWindow, mouseButtonCallback);
	glfwSetCursorPosCallback(mWindow, cursorPosCallback);
	glfwSetScrollCallback(mWindow, mouseScrollCallback);
}

Window::~Window()
{
	glfwDestroyWindow(mWindow);
}

void* Window::getWindowPtr()
{
	return glfwGetWin32Window(mWindow);;
}

int Window::getWidth()
{
	return mWidth;
}

int Window::getHeight()
{
	return mHeight;
}

void Window::resize(uint32_t width, uint32_t height)
{
	mWidth = width;
	mHeight = height;
	if(mResizeFunc)
        mResizeFunc();
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(mWindow);
}

void Window::setResizeFunc(std::function<void()> func)
{
    mResizeFunc = func;
}

void Window::update()
{
    glfwPollEvents();
}

void resizeCallback(GLFWwindow * window, int width, int height)
{
	Window * win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	win->resize(width, height);
}

void cursorPosCallback(GLFWwindow * window, double xPos, double yPos)
{
	Input::instance().setMousePosition(glm::vec2(xPos, yPos));
}

void keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	int curKey = 0;
	if (key >= 48 && key <= 57) 
	{
		curKey = key - 48;
	}
	else if (key >= 65 && key <= 90) 
	{
		curKey = key - 55;
	}
	else 
	{
		return;
	}

	switch (action) 
	{
	case GLFW_PRESS:
		Input::instance().setKeyDown(curKey);
		Input::instance().setKey(curKey, true);
		break;
	case GLFW_RELEASE:
		Input::instance().setKeyUp(curKey);
		Input::instance().setKey(curKey, false);
		break;
	default:
		break;
	}
}

void mouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{
	switch (action) 
	{
	case GLFW_PRESS:
		Input::instance().setMouseButtonDown(button);
		Input::instance().setMouseButton(button, true);
		break;
	case GLFW_RELEASE:
		Input::instance().setMouseButtonUp(button);
		Input::instance().setMouseButton(button, false);
		break;
	default:
		break;
	}
}

void mouseScrollCallback(GLFWwindow * window, double xOffset, double yOffset)
{
	Input::instance().setMouseScrollWheel((float)yOffset);
}


