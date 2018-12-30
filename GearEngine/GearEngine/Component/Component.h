#ifndef COMPONENT_H
#define COMPONENT_H
#include <string>
#include "../SceneGraph/Node.h"

class Component : public std::enable_shared_from_this<Component>
{
public:
	Component();
	~Component();
	void destroy();
	bool getEnabled();
	void setEnabled(bool enable);
	bool getDestroyed();
	int getID();
	void setID(int id);
	std::shared_ptr<Node> getNode();
	void setNode(std::shared_ptr<Node> node);
	std::string getType();
private:
	int mID;
	std::string mType;
	std::shared_ptr<Node> mNode;
	bool mEnabled;
	bool mDestroyed;
};

#endif