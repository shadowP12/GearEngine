#ifndef APPLICATION_H
#define APPLICATION_H
#include "Window.h"

class Application
{
public:
	Application(uint32_t width, uint32_t height);
	virtual ~Application();
	virtual void prepare();
    virtual void run();
	virtual void finish();
    virtual void runMainLoop();
protected:
	Window* mWindow;
};
#endif