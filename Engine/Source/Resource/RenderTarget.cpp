#include "RenderTarget.h"
#include "Texture.h"
#include <Blast/Gfx/GfxTexture.h>

namespace gear {
    RenderTarget::RenderTarget() {
        for (uint32_t i = 0; i < TARGET_COUNT; ++i) {
            std::get<0>(_colors[i]) = nullptr;
            std::get<1>(_colors[i]) = 0;
            std::get<2>(_colors[i]) = 0;
        }
        std::get<0>(_depth_stencil) = nullptr;
        std::get<1>(_depth_stencil) = 0;
        std::get<2>(_depth_stencil) = 0;
    }

    RenderTarget::~RenderTarget() {
    }

    void RenderTarget::SetColor(uint32_t idx, Texture* texture, uint32_t layer, uint32_t level) {
        _colors[idx] = std::tuple<Texture*, uint32_t, uint32_t>(texture, layer, level);
    }

    std::tuple<Texture*, uint32_t, uint32_t> RenderTarget::GetColor(uint32_t idx) {
        return _colors[idx];
    }

    void RenderTarget::SetDepthStencil(Texture* texture, uint32_t layer, uint32_t level) {
        _depth_stencil = std::tuple<Texture*, uint32_t, uint32_t>(texture, layer, level);
    }

    std::tuple<Texture*, uint32_t, uint32_t> RenderTarget::GetDepthStencil() {
        return _depth_stencil;
    }
}