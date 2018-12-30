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
	void destroyed();
	bool getActive();
	bool getActiveInHierarchy();
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