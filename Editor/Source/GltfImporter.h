#pragma once

#include <vector>
#include <string>

#include <Renderer/RenderData.h>

namespace gear {
    class Texture;
    class VertexBuffer;
    class IndexBuffer;
    class Entity;
    class Material;
    class MaterialInstance;
    class Skeleton;
    class AnimationClip;
}

struct GltfAsset {
    std::vector<gear::Texture*> textures;
    std::vector<gear::VertexBuffer*> vertex_buffers;
    std::vector<gear::IndexBuffer*> index_buffers;
    std::vector<gear::Material*> materials;
    std::vector<gear::MaterialInstance*> material_instances;
    std::vector<gear::Entity*> entities;
    std::vector<gear::Skeleton*> skeletons;
    std::vector<gear::AnimationClip*> animation_clips;
};

struct alignas(4) GltfMaterialConfig {
    gear::BlendStateType blending_mode;
    bool has_base_color_tex = false;
    bool has_normal_tex = false;
    bool has_metallic_roughness_tex = false;
    struct Eq {
        bool operator()(const GltfMaterialConfig& c0, const GltfMaterialConfig& c1) const;
    };
};

GltfAsset* ImportGltfAsset(const std::string&);

void DestroyGltfAsset(GltfAsset* asset);