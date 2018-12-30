#include "EntityManager.h"

EntityManager::EntityManager()
{
}

EntityManager::~EntityManager()
{
}

void EntityManager::updata()
{
	for (EntityMap::iterator iter = mEntities.begin(); iter != mEntities.end(); )
	{
		if (iter == mEntities.end())
			break;
		if (iter->second->getDestroyed())
			mEntities.erase(iter);
		//todo:destroy event
	}
}

std::shared_ptr<Entity> EntityManager::createEntity(std::string name)
{
	std::shared_ptr<Entity> res = std::shared_ptr<Entity>(new Entity());
	res->setName(name);
	std::pair<EntityMap::iterator, bool> result;
	result = mEntities.insert(EntityMap::value_type(res->getName(), res));
	if (result.second)
	{
		//Entity has already exist!
		return res;
	}
	return NULL;
}
