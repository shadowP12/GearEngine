#ifndef BASE_APPLICATION_H
#define BASE_APPLICATION_H

#include "BasePrerequisites.h"
typedef std::map<std::string, std::shared_ptr<System>> SystemMap;
class BaseApplication
{
public:
	BaseApplication();
	~BaseApplication();
	void baseUpdata();
	template<class ComponentType>
	std::shared_ptr<ComponentType> createComponent();
private:
	void addSystem(std::string type, std::shared_ptr<System> sys);
private:
	std::shared_ptr<EntityManager> mEntityManager;
	SystemMap mSystems;
};

#endif