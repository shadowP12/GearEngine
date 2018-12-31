#ifndef APPLICATION_H
#define APPLICATION_H
#include "../Window/Window.h"
#include "BaseApplication.h"

class Application : public BaseApplication
{
public:
	Application();
	~Application();
	void runMainLoop();
private:
	std::shared_ptr<Window> mWindow;
};
#endif