#include "Application.h"
#include "RHI/RHI.h"

Application::Application()
{
	RHI::startUp();
	Input::startUp();
	mWindow = new Window(800,600);
	
}

Application::~Application()
{
	if (mWindow)
		delete mWindow;
	Input::shutDown();
	RHI::shutDown();
}

void Application::runMainLoop()
{
	while (!glfwWindowShouldClose(mWindow->getWindowPtr())) 
	{
		Input::instance().update();
		glfwPollEvents();
	}
}
