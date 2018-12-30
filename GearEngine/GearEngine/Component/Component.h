#ifndef COMPONENT_H
#define COMPONENT_H
#include <string>
class Entity;
class Component
{
public:
	Component();
	~Component();
	bool getEnabled();
	void setEnabled();
	
private:
	std::string mType;
	std::shared_ptr<Entity> mEntity;
	bool mEnabled;
	bool mDestroyed;
};


#endif