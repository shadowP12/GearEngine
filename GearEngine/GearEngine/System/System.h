#ifndef SYSTEM_H
#define SYSTEM_H
#include <map>
#include "../Component/Component.h"

typedef std::map<int, std::shared_ptr<Component>> S_ComponentMap;
class System
{
public:
	System();
	virtual ~System();
	virtual void initSystem() = 0;
	virtual void addComponent(std::shared_ptr<Component> com) = 0;
	virtual void updata() = 0;
private:
	void updataComponentState();
public:
	int mNextID;
	S_ComponentMap mComponents;
};

#endif
