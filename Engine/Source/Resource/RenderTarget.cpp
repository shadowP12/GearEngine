//#include "RenderTarget.h"
//#include "Renderer.h"
//#include <Blast/Gfx/GfxTexture.h>
//
//namespace gear {
//    RenderTarget::RenderTarget(Renderer* renderer) {
//        mRenderer = renderer;
//        mOffscreen = false;
//    }
//
//    RenderTarget::RenderTarget(Renderer* renderer, const RenderTargetDesc& desc) {
//        mRenderer = renderer;
//        mOffscreen = true;
//        mWidth = desc.width;
//        mHeight = desc.height;
//        mSamples = desc.samples;
//        for (int i = 0; i < TARGET_COUNT; ++i) {
//            if (!desc.color[i].texture) {
//                continue;
//            }
//            mColor[i] = desc.color[i];
//        }
//        if (desc.depthStencil.texture) {
//            mDepthStencil = desc.depthStencil;
//        }
//    }
//
//    RenderTarget::~RenderTarget() {
//    }
//
//    uint32_t RenderTarget::getWidth() {
//        if (!mOffscreen) {
//            return mRenderer->getColor().texture->getWidth();
//        }
//        return mWidth;
//    }
//
//    uint32_t RenderTarget::getHeight() {
//        if (!mOffscreen) {
//            return mRenderer->getColor().texture->getHeight();
//        }
//        return mHeight;
//    }
//
//    uint32_t RenderTarget::getColorTargetCount() {
//        if (!mOffscreen) {
//            return 1;
//        }
//        uint32_t count = 0;
//        for (int i = 0; i < TARGET_COUNT; i++) {
//            if (mColor[i].texture) {
//                count++;
//            }
//        }
//        return count;
//    }
//
//    Attachment RenderTarget::getColor(int idx) {
//        if (mOffscreen) {
//            return mColor[idx];
//        }
//        return mRenderer->getColor();
//    }
//
//    bool RenderTarget::hasDepthStencil() {
//        if (mOffscreen) {
//            return mDepthStencil.texture != nullptr;
//        }
//        return mRenderer->getDepthStencil().texture != nullptr;
//    }
//
//    Attachment RenderTarget::getDepthStencil() {
//        if (mOffscreen) {
//            return mDepthStencil;
//        }
//        return mRenderer->getDepthStencil();
//    }
//}