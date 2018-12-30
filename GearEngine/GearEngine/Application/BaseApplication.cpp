#include "BaseApplication.h"

BaseApplication::BaseApplication()
{
}

BaseApplication::~BaseApplication()
{
}

void BaseApplication::baseUpdata()
{
	//updata systems
	for (SystemMap::iterator iter = mSystems.begin(); iter != mSystems.end(); )
	{
		if (iter == mSystems.end())
			break;
		iter->second->updata();
	}
	//updata entities
	mEntityManager->updata();
}

void BaseApplication::addSystem(std::string type,std::shared_ptr<System> sys)
{
	std::pair<SystemMap::iterator, bool> result;
	result = mSystems.insert(SystemMap::value_type(type, sys));
	if (result.second)
	{
		//toso
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