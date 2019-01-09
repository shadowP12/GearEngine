#ifndef MATERIAL_MANAGER_H
#define MATERIAL_MANAGER_H
#include "ResourceManager.h"
#include "../Utility/Module.h"

class MaterialManager : public ResourceManager, public Module<MaterialManager>
{
public:
	MaterialManager();
	~MaterialManager();

private:

};

MaterialManager::MaterialManager()
{
}

MaterialManager::~MaterialManager()
{
}
#endif