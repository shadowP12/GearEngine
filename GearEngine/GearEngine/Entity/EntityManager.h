#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H
#include "Entity.h"

typedef std::map<std::string, std::shared_ptr<Entity>> EntityMap;

class EntityManager
{
public:
	EntityManager();
	virtual ~EntityManager();
	void updata();
	std::shared_ptr<Entity> createEntity(std::string name);
private:
	EntityMap mEntities;
};

#endif