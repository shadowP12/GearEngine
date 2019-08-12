#include <iostream>
#include "Application/Application.h"
int main()
{
	Application app;
	app.prepare();
	app.runMainLoop();
	return 0;
}