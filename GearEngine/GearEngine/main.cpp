#include<iostream>
#include <memory>
#include"Entity/Entity.h"
int main() {
	std::shared_ptr<Entity> e1 = std::shared_ptr<Entity>(new Entity());
	std::shared_ptr<Entity> e2 = std::shared_ptr<Entity>(new Entity());
	e1->appendChildren(e2);
	e1->destroy();
	return 0;
}