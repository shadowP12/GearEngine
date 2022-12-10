#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include <Renderer/RenderData.h>

namespace gear {
    class Entity;
    class Mesh;
    class Material;
    class MaterialInstance;
    class Skeleton;
    class AnimationClip;
}

struct GltfAsset {
    std::unordered_map<std::string, std::shared_ptr<blast::GfxTexture>> textures;
    std::unordered_map<std::string, std::shared_ptr<gear::Mesh>> meshs;
    std::unordered_map<std::string, std::shared_ptr<gear::Material>> materials;
    std::unordered_map<std::string, std::shared_ptr<gear::MaterialInstance>> material_instances;
    std::unordered_map<std::string, std::shared_ptr<gear::Skeleton>> skeletons;
    std::unordered_map<std::string, std::shared_ptr<gear::AnimationClip>> animation_clips;
    std::unordered_map<std::string, std::shared_ptr<gear::Entity>> entities;
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