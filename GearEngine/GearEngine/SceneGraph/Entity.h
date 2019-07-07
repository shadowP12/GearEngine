#ifndef ENTITY_H
#define ENTITY_H
#include <map>
#include "../SceneGraph/Node.h"
#include "../Component/Component.h"

typedef std::map<std::string, std::shared_ptr<Component>> ComponentMap;

class Entity : public Node
{
public:
	Entity();
	~Entity();
	void activate();
	void deactivate();
	void destroy();
	bool getDestroyed();
	bool getActive();
	bool getActiveInHierarchy();
	std::string getName();
	void setName(std::string name);
	std::shared_ptr<Component> getComponent(std::string type);
	void addComponent(std::shared_ptr<Component> newComponent);
	void removeComponent(std::string type);
private:
	void notifyChildren(bool active);
public:
	std::string mName;
private:
	ComponentMap mComponents;
	bool mDestroyed;
	bool mActive;
	bool mAncestorActive;
};


#endif