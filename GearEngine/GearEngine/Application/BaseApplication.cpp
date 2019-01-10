#include "BaseApplication.h"

BaseApplication::BaseApplication()
{
	mEntityManager = std::shared_ptr<EntityManager>(new EntityManager());
}

BaseApplication::~BaseApplication()
{
}

void BaseApplication::EcsUpdate()
{
	//update systems
	for (SystemMap::iterator iter = mSystems.begin(); iter != mSystems.end(); )
	{
		if (iter == mSystems.end())
			break;
		iter->second->update();
	}
	//update entities
	mEntityManager->update();
}

void BaseApplication::addSystem(std::string type,std::shared_ptr<System> sys)
{
	std::pair<SystemMap::iterator, bool> result;
	result = mSystems.insert(SystemMap::value_type(type, sys));
	if (result.second)
	{
		//todo
	}
}

template<class ComponentType>
std::shared_ptr<ComponentType> BaseApplication::createComponent() 
{
	std::shared_ptr<ComponentType> res = std::shared_ptr<ComponentType>(new ComponentType());

	SystemMap::iterator typeIt = mSystems.find(res->getType());
	if (typeIt != mSystems.end())
	{
		typeIt->second->addComponent(res);
		return res;
	}
	return NULL;
}