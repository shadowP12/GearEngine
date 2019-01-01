#include "Application.h"
#include "../RenderAPI/VulkanContext.h"

Application::Application()
{
	mWindow = std::shared_ptr<Window>(new Window(800,600));
	mWindow->initWindow();
	VulkanContext::startUp(mWindow->getWindowPtr());
}

Application::~Application()
{
	VulkanContext::shutDown();
}

void Application::runMainLoop()
{
	while (!glfwWindowShouldClose(mWindow->getWindowPtr())) 
	{
		glfwPollEvents();
	}
}
