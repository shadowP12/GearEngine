#ifndef APPLICATION_H
#define APPLICATION_H
#include "../Window/Window.h"

class Application
{
public:
	Application();
	~Application();
	void runMainLoop();
private:
	std::shared_ptr<Window> mWindow;
};
#endif