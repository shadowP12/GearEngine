#include "Application.h"
#include "Utility/FileSystem.h"

Application::Application(uint32_t width, uint32_t height)
{
	Input::startUp();
	mWindow = new Window(width, height);
}

Application::~Application()
{
	if (mWindow)
		delete mWindow;
	Input::shutDown();
}

void Application::prepare()
{
}

void Application::run()
{
}

void Application::finish()
{
}

void Application::runMainLoop()
{
    while (!mWindow->shouldClose())
    {
        run();
        Input::instance().update();
        mWindow->update();
    }
}

