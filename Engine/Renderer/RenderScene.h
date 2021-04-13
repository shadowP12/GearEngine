#pragma once
#include "Core/GearDefine.h"
#include "RenderTarget.h"
#include "Math/Math.h"
#include <Blast/Gfx/GfxBuffer.h>
#include <Blast/Gfx/GfxTexture.h>
#include <Blast/Gfx/GfxSampler.h>
#include <Blast/Gfx/GfxShader.h>
#include <map>
#include <vector>

namespace gear {
    struct RenderPrimitive {
        Blast::GfxBuffer* vertexBuffer = nullptr;
        Blast::GfxBuffer* indexBuffer = nullptr;
        Blast::GfxShader* vertexShader = nullptr;
        Blast::GfxShader* pixelShader = nullptr;
        uint32_t offset = 0;
    };

    class RenderView {
    public:
        RenderView(Renderer* renderer);
        ~RenderView();
        void prepare();
        RenderTarget* getRenderTarget() { return mRenderTarget; }
        void setRenderTarget(RenderTarget* renderTarget) { mRenderTarget = renderTarget; }
        Blast::GfxBuffer* getViewBuffer() { return mViewBuffer; }
        struct ViewBufferDesc {
            glm::mat4 viewMatrix;
            glm::mat4 projMatrix;
        };
        ViewBufferDesc& getViewBufferDesc() { return mViewBufferDesc; }
    private:
        Renderer* mRenderer = nullptr;
        RenderTarget* mRenderTarget = nullptr;
        Blast::GfxBuffer* mViewBuffer = nullptr;
        ViewBufferDesc mViewBufferDesc = {};
    };

    class RenderScene {
    public:
        RenderScene(Renderer* renderer);
        ~RenderScene();
        RenderView* genView();
        RenderPrimitive* genPrimitive();
        void deleteView(RenderView* view);
        void deletePrimitive(RenderPrimitive* primitive);
    private:
        friend Renderer;
        Renderer* mRenderer = nullptr;
        std::vector<RenderView*> mViews;
        std::vector<RenderPrimitive*> mPrimitives;
    };
}