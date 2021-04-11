#include "Resource/ResourceManager.h"
#include "Resource/Mesh.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Utility/Log.h"
#include "Utility/UUID.h"

namespace gear {
    ResourceManager::ResourceManager() {
    }

    ResourceManager::~ResourceManager() {
    }

    std::vector<std::shared_ptr<Resource>> ResourceManager::findRes(const std::string& id) {
        std::vector<std::shared_ptr<Resource>> findResults;
        for (auto& var : mResources) {
            if (var.second->getId() == id) {
                findResults.push_back(var.second);
            }
        }
        return findResults;
    }

    std::shared_ptr<Resource> ResourceManager::getRes(const std::string& uuid) {
        std::shared_ptr<Resource> res;
        std::map<std::string, std::shared_ptr<Resource>>::iterator it = mResources.find(uuid);
        if (it != mResources.end()) {
            res = it->second;
        }
        return res;
    }

    void ResourceManager::addRes(std::shared_ptr<Resource> res) {
        std::pair<std::map<std::string, std::shared_ptr<Resource>>::iterator, bool> result;
        result = mResources.insert(std::map<std::string, std::shared_ptr<Resource>>::value_type(res->mUUID, res));

        if (!result.second) {
            LOGW("Res has already exist!\n");
        }
    }

    void ResourceManager::removeRes(std::shared_ptr<Resource> res) {
        std::map<std::string, std::shared_ptr<Resource>>::iterator it = mResources.find(res->mUUID);
        if (it != mResources.end()) {
            mResources.erase(it);
        }
    }

    std::shared_ptr<Mesh> ResourceManager::createMesh(const std::string& id, const MeshDesc& desc) {
        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(desc);
        mesh->mId = id;
        mesh->mUUID = generateUUID();
        addRes(mesh);
        return mesh;
    }

    std::shared_ptr<Texture> ResourceManager::createTexture(const std::string& id, const TextureDesc& desc) {
        std::shared_ptr<Texture> texture = std::make_shared<Texture>(desc);
        texture->mId = id;
        texture->mUUID = generateUUID();
        addRes(texture);
        return texture;
    }

    std::shared_ptr<Material> ResourceManager::createMaterial(const std::string& id, const MaterialDesc& desc) {
        if (desc.type == MaterialType::PBR) {
            std::shared_ptr<PBRMaterial> material = std::make_shared<PBRMaterial>();
            material->mId = id;
            material->mUUID = generateUUID();
            addRes(material);
            return material;
        }
        return nullptr;
    }

}