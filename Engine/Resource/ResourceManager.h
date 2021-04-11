#pragma once
#include "Core/GearDefine.h"
#include "Resource/Resource.h"

#include <vector>
#include <map>
#include <memory>
namespace gear {

    struct MeshDesc;
    class Mesh;
    struct TextureDesc;
    class Texture;
    struct MaterialDesc;
    class Material;

    class ResourceManager {
    public:
        ResourceManager();
        ~ResourceManager();
        const std::map<std::string, std::shared_ptr<Resource>>& getAllRes() { return mResources; }
        std::vector<std::shared_ptr<Resource>> findRes(const std::string& id);
        std::shared_ptr<Resource> getRes(const std::string& uuid);
        void addRes(std::shared_ptr<Resource> res);
        void removeRes(std::shared_ptr<Resource> res);
        std::shared_ptr<Mesh> createMesh(const std::string& id, const MeshDesc& desc);
        std::shared_ptr<Texture> createTexture(const std::string& id, const TextureDesc& desc);
        std::shared_ptr<Material> createMaterial(const std::string& id, const MaterialDesc& desc);
    private:
        std::map<std::string, std::shared_ptr<Resource>> mResources;
    };

}