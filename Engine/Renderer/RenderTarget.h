#pragma once
#include "Core/GearDefine.h"
#include <Blast/Gfx/GfxDefine.h>

namespace Blast {
    class GfxContext;
    class GfxTexture;
}

namespace gear {
    class Renderer;
    struct Attachment {
        Blast::Format format;
        Blast::GfxTexture* texture = nullptr;
        uint32_t layer;
        uint32_t level;
    };

    struct RenderTargetDesc {
        uint32_t width;
        uint32_t height;
        Blast::SampleCount samples = Blast::SAMPLE_COUNT_1;
        Attachment color[TARGET_COUNT];
        Attachment depthStencil;
    };

    class RenderTarget {
    public:
        RenderTarget(Renderer* renderer);
        RenderTarget(Renderer* renderer, const RenderTargetDesc& desc);
        ~RenderTarget();
        uint32_t getWidth();
        uint32_t getHeight();
        Blast::SampleCount getSamples() { return mSamples; }
        uint32_t getColorTargetCount();
        Attachment getColor(int idx);
        Attachment getDepthStencil();
        bool hasDepthStencil();
    private:
        Renderer* mRenderer = nullptr;
        bool mOffscreen = false;
        uint32_t mWidth;
        uint32_t mHeight;
        Blast::SampleCount mSamples;
        Attachment mColor[TARGET_COUNT] = {};
        Attachment mDepthStencil = {};
    };
}
