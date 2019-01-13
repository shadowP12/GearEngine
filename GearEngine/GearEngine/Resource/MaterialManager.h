#ifndef MATERIAL_MANAGER_H
#define MATERIAL_MANAGER_H
#include "ResourceManager.h"
#include "../Utility/Module.h"
#include "../Material/Material.h"

class MaterialManager : public ResourceManager, public Module<MaterialManager>
{
public:
	MaterialManager();
	~MaterialManager();
	std::shared_ptr<Material> getRes(std::string name)
	{
		return std::dynamic_pointer_cast<Material>(getResImp(name));
	}
private:

};

MaterialManager::MaterialManager()
{
}

MaterialManager::~MaterialManager()
{
}
#endif