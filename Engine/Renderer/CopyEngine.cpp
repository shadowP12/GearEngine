#include "CopyEngine.h"
#include "Renderer.h"
#include <Gfx/GfxContext.h>
#include <Gfx/GfxCommandBuffer.h>

namespace gear {
    CopyEngine::CopyEngine(Renderer* renderer) {
        mRenderer = renderer;
    }

    CopyEngine::~CopyEngine() {
        mRenderer->mQueue->waitIdle();
        for (int i = 0; i < mSets.size(); ++i) {
            SAFE_DELETE(mSets[i]->fence);
            SAFE_DELETE(mSets[i]->cmd);
            SAFE_DELETE(mSets[i]);
        }
        mSets.clear();
    }

    CopySet* CopyEngine::getActiveSet() {
        for (int i = 0; i < mSets.size(); ++i) {
            if (mSets[i]->fence->getFenceStatus() == Blast::FENCE_STATUS_COMPLETE) {
                return mSets[i];
            }
        }
        CopySet* newSet = new CopySet();
        newSet->fence = mRenderer->mContext->createFence();
        newSet->cmd = mRenderer->mCmdPool->allocBuf(false);
        mSets.push_back(newSet);
        return newSet;
    }

}