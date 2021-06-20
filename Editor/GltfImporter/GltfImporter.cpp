#include "GltfImporter.h"
#include <Engine/Utility/Hash.h>
#include <unordered_map>

bool GltfMaterialConfig::Eq::operator()(const GltfMaterialConfig& c0, const GltfMaterialConfig& c1) const {
    if (c0.blendingMode != c1.blendingMode) return false;
    if (c0.hasBaseColorTex != c1.hasBaseColorTex) return false;
    if (c0.hasNormalTex != c1.hasNormalTex) return false;
    if (c0.hasmetallicRoughnessTex != c1.hasmetallicRoughnessTex) return false;
    return true;
}

GltfImporter::GltfImporter() {

}

GltfImporter::~GltfImporter() {

}

GltfAsset GltfImporter::import(const std::string&) {
    // 初始化资源容器
    std::vector<gear::Texture*> textures;
    std::vector<gear::VertexBuffer*> vertexBuffers;
    std::vector<gear::IndexBuffer*> indexBuffers;
    std::vector<gear::Material*> materiale;
    std::vector<gear::MaterialInstance*> materialInstances;
    std::vector<gear::Entity*> Entities;
    std::unordered_map<GltfMaterialConfig, gear::Material*, MurmurHash<GltfMaterialConfig>, GltfMaterialConfig::Eq> materialMap;

    GltfAsset asset;
    return asset;
}