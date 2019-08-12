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
	RHIDevice* device = RHI::instance().getDevice();

	RHIProgramInfo programInfo;
	programInfo.type = RHIProgramType::Vertex;
	programInfo.entryPoint = "main";
	readFile("D:/GearEngine/GearEngine/Resource/Shaders/default.vert", programInfo.source);
}



void Application::runMainLoop()
{
	while (!glfwWindowShouldClose(mWindow->getWindowPtr())) 
	{
		Input::instance().update();
		glfwPollEvents();
	}
}
