#ifndef MESH_MANAGER_H
#define MESH_MANAGER_H
#include "ResourceManager.h"
#include "../Utility/Module.h"
#include "../Mesh/Mesh.h"

class MeshManager : public ResourceManager, public Module<MeshManager>
{
public:
	MeshManager()
	{}
	virtual ~MeshManager()
	{}
	std::shared_ptr<Mesh> getRes(std::string name)
	{
		return std::dynamic_pointer_cast<Mesh>(getResImp(name));
	}
private:

};
#endif