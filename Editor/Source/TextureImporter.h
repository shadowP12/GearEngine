#pragma once
#include <string>

namespace gear {
    class Texture;
}

gear::Texture* ImportTexture2D(const std::string&);

gear::Texture* ImportTexture2DWithFloat(const std::string&);