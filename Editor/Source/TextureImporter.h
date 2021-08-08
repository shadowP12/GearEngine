#pragma once
#include <string>

namespace gear {
    class Texture;
}

class TextureImporter {
public:
    TextureImporter();

    ~TextureImporter();

    gear::Texture* importTexture2D(const std::string&);
};