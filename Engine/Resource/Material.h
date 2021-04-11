#pragma once

#include "Core/GearDefine.h"
#include "Math/Math.h"
#include "Resource/Resource.h"

#include <string>
#include <unordered_map>

namespace gear {
    class Texture;

    enum class MaterialType {
        None,
        PBR
    };

    struct MaterialDesc {
        MaterialType type;
    };

    class Material : public Resource {
    public:
        Material() {}
        ~Material() {}
    protected:
        ResourceType mType = ResourceType::MATERIAL;
        MaterialType mMaterialType = MaterialType::None;
    };

    class PBRMaterial : public Material {
    public:
        PBRMaterial() {}
        ~PBRMaterial() {}
    protected:
        MaterialType mMaterialType = MaterialType::PBR;
    };
}