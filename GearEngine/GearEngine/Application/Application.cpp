#include "Application.h"
#include "../RenderAPI/VulkanContext.h"
#include "../Input/Input.h"
#include "../Resource/MeshManager.h"
#include "../Resource/TextureManager.h"
#include "../Renderer/Renderer.h"
#include "../Importer/ImporterManager.h"

Application::Application()
{
	Input::startUp();
	mWindow = std::shared_ptr<Window>(new Window(800,600));
	mWindow->initWindow();
	VulkanContext::startUp(mWindow->getWindowPtr());

	//renderer
	Renderer::startUp();
	Renderer::instance().init(mWindow->getWidth(),mWindow->getHeight());

	//resource manager
	MeshManager::startUp();
	TextureManager::startUp();

	//Shader manager

	//Importer
	ImporterManager::startUp();
}

Application::~Application()
{
	ImporterManager::shutDown();
	TextureManager::shutDown();
	MeshManager::shutDown();
	Renderer::shutDown();
	VulkanContext::shutDown();
	Input::shutDown();
}

void Application::runMainLoop()
{
	while (!glfwWindowShouldClose(mWindow->getWindowPtr())) 
	{
		if (mWindow->getReSize())
		{
			if (mWindow->getWidth() == 0)
			{
				//stop
			}
			else
			{
				Renderer::instance().reSize(mWindow->getWidth(), mWindow->getHeight());
				mWindow->setReSize(false);
			}
		}
		else
		{
			Renderer::instance().draw();
		}
		Input::instance().update();
		glfwPollEvents();
		
	}
}
