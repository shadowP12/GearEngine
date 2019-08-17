#ifndef APPLICATION_H
#define APPLICATION_H
#include "Window.h"

class Application
{
public:
	Application();
	~Application();
	void prepare();
	void runMainLoop();
private:
	Window* mWindow;
};
#endif