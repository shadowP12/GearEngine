#include "Application.h"
#include "../RenderAPI/VulkanContext.h"
#include "../Input/Input.h"
#include "../Resource/MeshManager.h"
#include "../Resource/TextureManager.h"
#include "../Resource/MaterialManager.h"
#include "../Renderer/Renderer.h"

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
	MaterialManager::startUp();
}

Application::~Application()
{
	MaterialManager::shutDown();
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
		Input::instance().update();
		if (mWindow->getReSize())
		{
			Renderer::instance().reSize(mWindow->getWidth(), mWindow->getHeight());
			mWindow->setReSize(false);
		}
		EcsUpdate();
		glfwPollEvents();
		
	}
}
