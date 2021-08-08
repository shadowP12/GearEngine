#pragma once

#include <Engine/Utility/Flags.h>
#include <string>

namespace gear {
    class Material;
}

struct GltfMaterialConfig;

class GltfMaterialTemplate {
public:
    GltfMaterialTemplate();

    ~GltfMaterialTemplate();

    gear::Material* gen(GltfMaterialConfig* config);

private:
    bool replace(std::string& str, const std::string& from, const std::string& to);
};