#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H
#include "Resource.h"
#include "../Utility/Module.h"
#include <map>

class ResourceManager : public Module<ResourceManager>
{
public:
	ResourceManager();
	~ResourceManager();
	//load
	//add
	//remove
private:
	//map<uuid,res>
	std::map<std::string, std::shared_ptr<Resource>> mResources;
};

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}
#endif