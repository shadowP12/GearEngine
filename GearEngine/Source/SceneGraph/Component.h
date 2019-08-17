#ifndef COMPONENT_H
#define COMPONENT_H
#include <string>
#include "../SceneGraph/Node.h"

class Component : public std::enable_shared_from_this<Component>
{
public:
	Component();
	virtual ~Component();
	void destroy();
	virtual void destroyImp() = 0;
	virtual void setEnabledImp(bool enable) = 0;
	bool getEnabled();
	void setEnabled(bool enable);
	bool getDestroyed();
	int getID();
	void setID(int id);
	std::string getType();
protected:
	int mID;
	std::string mType;
	bool mEnabled;
	bool mDestroyed;
};

#endif