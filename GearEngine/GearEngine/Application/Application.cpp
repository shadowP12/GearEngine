#include "Application.h"
#include "../RenderAPI/VulkanContext.h"
#include "../Input/Input.h"
#include "../Resource/MeshManager.h"
#include "../Resource/TextureManager.h"
#include "../Resource/MaterialManager.h"
#include "../Renderer/Renderer.h"
#include "../Importer/ImporterManager.h"
#include "../Component/StaticModelComponent.h"//todo:...

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

	//Importer
	ImporterManager::startUp();
	ImporterManager::instance().loadTexture("Drone_diff.jpeg");
	ImporterManager::instance().loadMesh("scene.gltf");
	ImporterManager::instance().loadMaterial("default.json");

	//test
	std::shared_ptr<Entity> ent = mEntityManager->createEntity("test");

	std::shared_ptr<StaticModelComponent> smc = std::shared_ptr<StaticModelComponent>(new StaticModelComponent(MeshManager::instance().getRes("Suzanne"), MaterialManager::instance().getRes("default.json"), ent));
	ent->addComponent(smc);
}

Application::~Application()
{
	ImporterManager::shutDown();
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
		EcsUpdate();
		Input::instance().update();
		glfwPollEvents();
		
	}
}
