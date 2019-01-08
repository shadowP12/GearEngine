#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H
#include "Resource.h"
#include <map>

class ResourceManager
{
public:
	ResourceManager();
	virtual ~ResourceManager();
	virtual std::shared_ptr<Resource> getRes(std::string name)
	{
		std::shared_ptr<Resource> res;
		std::map<std::string, std::shared_ptr<Resource>>::iterator nameIt = mResources.find(name);
		if (nameIt != mResources.end())
		{
			res = nameIt->second;
		}
		return res;
	}
	virtual void addRes(std::shared_ptr<Resource> res)
	{
		std::pair<std::map<std::string, std::shared_ptr<Resource>>::iterator, bool> result;
		result = mResources.insert(std::map<std::string, std::shared_ptr<Resource>>::value_type(res->mName, res));

		if (!result.second)
		{
			std::cout << "Res has already exist!" << std::endl;
		}
	}

	virtual void removeRes(std::shared_ptr<Resource> res)
	{
		std::map<std::string, std::shared_ptr<Resource>>::iterator nameIt = mResources.find(res->mName);
		if (nameIt != mResources.end())
		{
			mResources.erase(nameIt);
		}
	}
private:
	std::map<std::string, std::shared_ptr<Resource>> mResources;
};

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}
#endif