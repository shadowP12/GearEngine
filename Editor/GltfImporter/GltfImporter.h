#pragma once

#include <vector>
#include <string>
#include <Engine/Resource/Material.h>

namespace gear {
    class Texture;
    class VertexBuffer;
    class IndexBuffer;
    class Entity;
}

struct GltfAsset {
    std::vector<gear::Texture*> textures;
    std::vector<gear::VertexBuffer*> vertexBuffers;
    std::vector<gear::IndexBuffer*> indexBuffers;
    std::vector<gear::Material*> materiale;
    std::vector<gear::MaterialInstance*> materialInstances;
    std::vector<gear::Entity*> Entities;
};

struct alignas(4) GltfMaterialConfig {
    gear::BlendingMode blendingMode;
    bool hasBaseColorTex = false;
    bool hasNormalTex = false;
    bool hasmetallicRoughnessTex = false;
    struct Eq {
        bool operator()(const GltfMaterialConfig& c0, const GltfMaterialConfig& c1) const;
    };
};

class GltfImporter {
public:
    GltfImporter();

    ~GltfImporter();

    GltfAsset import(const std::string&);
};