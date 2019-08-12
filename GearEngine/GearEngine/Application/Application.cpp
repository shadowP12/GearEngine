#include "Application.h"
#include "RHI/RHI.h"
#include "RHI/Managers/RHIProgramManager.h"
#include "Utility/FileSystem.h"

Application::Application()
{
	RHI::startUp();
	Input::startUp();
	RHIProgramManager::startUp();
	mWindow = new Window(800,600);
	
}

Application::~Application()
{
	if (mWindow)
		delete mWindow;
	RHIProgramManager::shutDown();
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

	RHIProgram* program = device->createProgram(programInfo);
	program->compile();
	delete program;
}



void Application::runMainLoop()
{
	while (!glfwWindowShouldClose(mWindow->getWindowPtr())) 
	{
		Input::instance().update();
		glfwPollEvents();
	}
}
