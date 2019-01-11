#ifndef IMPORTER_MANAGER_H
#define IMPORTER_MANAGER_H

#include "TextureImporter.h"
#include "MeshImporter.h"
#include "MaterialImporter.h"
#include "../Utility/Module.h"

class ImporterManager : public Module<ImporterManager>
{
public:
	ImporterManager();
	~ImporterManager();
	void loadTexture(std::string name)
	{
		mTextureImporter->load(name);
	}
	void loadMesh(std::string name)
	{
		mMeshImporter->load(name);
	}
	void loadMaterial(std::string name)
	{
		mMaterialImporter->load(name);
	}
private:
	std::shared_ptr<MeshImporter> mMeshImporter;
	std::shared_ptr<TextureImporter> mTextureImporter;
	std::shared_ptr<MaterialImporter> mMaterialImporter;
};

ImporterManager::ImporterManager()
{
}

ImporterManager::~ImporterManager()
{
}
#endif