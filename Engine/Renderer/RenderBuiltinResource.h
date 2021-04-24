#pragma once

namespace Blast {
    class GfxBuffer;
    class GfxTexture;
    class GfxSampler;
    class GfxRootSignature;
    class GfxGraphicsPipeline;
};

namespace gear {
    class Renderer;
    class RenderBuiltinResource {
    public:
        RenderBuiltinResource(Renderer* renderer);
        ~RenderBuiltinResource();
        Blast::GfxBuffer* getQuadVertexBuffer() { return mQuadVertexBuffer; }
        Blast::GfxBuffer* getQuadIndexBuffer() { return mQuadIndexBuffer; }
    private:
        void createCustomRootSignature();
        void createQuadBuffer();
    private:
        friend Renderer;
        Renderer* mRenderer;
        Blast::GfxRootSignature* mCustomRootSignature = nullptr;
        Blast::GfxBuffer* mQuadVertexBuffer;
        Blast::GfxBuffer* mQuadIndexBuffer;
        Blast::GfxBuffer* mDefaultBuffer = nullptr;
        Blast::GfxTexture* mDefaultTexture = nullptr;
    };
}

