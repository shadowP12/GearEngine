#include "Application.h"
#include "RHI/RHI.h"
#include "Utility/FileSystem.h"

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

void Application::prepare()
{
}

void Application::runMainLoop()
{
	while (!glfwWindowShouldClose(mWindow->getWindowPtr())) 
	{
		mWindow->beginFrame();
		// ²åÈëÖ´ÐÐ´úÂë
		mWindow->endFrame();
		Input::instance().update();
		glfwPollEvents();
	}
}

void Application::finish()
{
}
