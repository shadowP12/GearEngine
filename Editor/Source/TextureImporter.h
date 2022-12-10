#pragma once
#include <string>
#include <memory>

namespace blast {
    class GfxTexture;
}

std::shared_ptr<blast::GfxTexture> ImportTexture2D(const std::string&);

std::shared_ptr<blast::GfxTexture> ImportTexture2DWithFloat(const std::string&);