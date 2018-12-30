#include<iostream>
#include <memory>
#include"Entity/Entity.h"
int main() {
	std::shared_ptr<Entity> e1 = std::shared_ptr<Entity>(new Entity());
	std::shared_ptr<Entity> e2 = std::shared_ptr<Entity>(new Entity());
	std::shared_ptr<Node> n1 = std::shared_ptr<Node>(new Node());
	return 0;
}