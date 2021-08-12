#pragma once
#include "Core/GearDefine.h"
#include <Blast/Gfx/GfxDefine.h>
#include <Blast/Gfx/GfxCommandBuffer.h>
#include <tuple>

namespace blast {
    class GfxContext;
    class GfxTexture;
}

namespace gear {
    class Renderer;
    class Texture;
    class RenderTarget {
    public:
        RenderTarget();

        ~RenderTarget();

        void Copy(RenderTarget* other);

        void SetWidth(uint32_t width);

        uint32_t GetWidth();

        void SetHeight(uint32_t height);

        uint32_t GetHeight();

        blast::SampleCount GetSamples() { return _sample_count; }

        void SetSampleCount(blast::SampleCount sample_count) { _sample_count = sample_count; }

        blast::ClearFlag GetClearFlag() { return (blast::ClearFlag)_clear_value.flags; }

        void SetClearFlag(blast::ClearFlag flag) { _clear_value.flags = flag; }

        void SetColor(uint32_t idx, Texture* texture, uint32_t layer, uint32_t level);

        std::tuple<Texture*, uint32_t, uint32_t> GetColor(uint32_t idx);

        void SetDepthStencil(Texture* texture, uint32_t layer, uint32_t level);

        std::tuple<Texture*, uint32_t, uint32_t> GetDepthStencil();

    private:
        uint32_t _width;
        uint32_t _height;
        blast::GfxClearValue _clear_value;
        blast::SampleCount _sample_count;
        std::tuple<Texture*, uint32_t, uint32_t> _colors[TARGET_COUNT];
        std::tuple<Texture*, uint32_t, uint32_t> _depth_stencil;
    };
}
