#ifndef MESH_MANAGER_H
#define MESH_MANAGER_H
#include "ResourceManager.h"
#include "../Utility/Module.h"

class MeshManager : public ResourceManager, public Module<MeshManager>
{
public:
	MeshManager();
	~MeshManager();

private:

};

MeshManager::MeshManager()
{
}

MeshManager::~MeshManager()
{
}
#endif