#include "Application.h"

Application::Application()
{
	mWindow = std::shared_ptr<Window>(new Window(800,600));
	mWindow->initWindow();
}

Application::~Application()
{
}

void Application::runMainLoop()
{
	while (!glfwWindowShouldClose(mWindow->getWindowPtr())) 
	{
		glfwPollEvents();
	}
}
