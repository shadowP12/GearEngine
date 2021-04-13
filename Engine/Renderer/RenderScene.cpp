#include "RenderScene.h"
#include "Renderer.h"
#include <Blast/Gfx/GfxContext.h>

namespace gear {
    RenderView::RenderView(Renderer* renderer) {
        mRenderer = renderer;
        Blast::GfxBufferDesc bufferDesc;
        bufferDesc.size = sizeof(ViewBufferDesc);
        bufferDesc.usage = Blast::RESOURCE_USAGE_CPU_TO_GPU;
        bufferDesc.type = Blast::RESOURCE_TYPE_UNIFORM_BUFFER;
        mViewBuffer = mRenderer->getContext()->createBuffer(bufferDesc);
    }

    RenderView::~RenderView() {
        SAFE_DELETE(mViewBuffer);
    }

    void RenderView::prepare() {
        mViewBuffer->writeData(0, sizeof(ViewBufferDesc), &mViewBufferDesc);
    }

    RenderScene::RenderScene(Renderer* renderer) {
        mRenderer = renderer;
    }

    RenderScene::~RenderScene() {
        for (int i = 0; i < mViews.size(); ++i) {
            SAFE_DELETE(mViews[i]);
        }
        mViews.clear();

        for (int i = 0; i < mPrimitives.size(); ++i) {
            SAFE_DELETE(mPrimitives[i]);
        }
        mPrimitives.clear();
    }

    RenderView* RenderScene::genView() {
        RenderView* view = new RenderView(mRenderer);
        mViews.push_back(view);
        return view;
    }

    RenderPrimitive* RenderScene::genPrimitive() {
        RenderPrimitive* primitive = new RenderPrimitive();
        mPrimitives.push_back(primitive);
        return primitive;
    }

    void RenderScene::deleteView(RenderView* view) {
        for (auto iter = mViews.begin(); iter != mViews.end(); ) {
            if (iter == mViews.end()) {
                break;
            }

            if (*iter == view) {
                SAFE_DELETE(view);
                mViews.erase(iter);
                break;
            }
        }
    }

    void RenderScene::deletePrimitive(RenderPrimitive* primitive) {
        for (auto iter = mPrimitives.begin(); iter != mPrimitives.end(); ) {
            if (iter == mPrimitives.end()) {
                break;
            }

            if (*iter == primitive) {
                SAFE_DELETE(primitive);
                mPrimitives.erase(iter);
                break;
            }
        }
    }



}