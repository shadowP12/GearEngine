#ifndef APPLICATION_H
#define APPLICATION_H
#include "Window.h"

class Application
{
public:
	Application();
	virtual ~Application();
	virtual void prepare();
	virtual void runMainLoop();
	virtual void finish();
protected:
	Window* mWindow;
};
#endif