#include "CopyEngine.h"
#include "Renderer.h"
#include <Blast/Gfx/GfxBuffer.h>
#include <Blast/Gfx/GfxContext.h>
#include <Blast/Gfx/GfxCommandBuffer.h>

namespace gear {
    CopyEngine::CopyEngine(Renderer* renderer) {
        mRenderer = renderer;
    }

    CopyEngine::~CopyEngine() {
        mRenderer->mQueue->waitIdle();
        for (int i = 0; i < mCommands.size(); ++i) {
            SAFE_DELETE(mCommands[i]->fence);
            SAFE_DELETE(mCommands[i]->cmd);
            SAFE_DELETE(mCommands[i]);
        }
        mCommands.clear();
    }

    CopyCommand* CopyEngine::getActiveCommand() {
        for (int i = 0; i < mCommands.size(); ++i) {
            if (mCommands[i]->fence->getFenceStatus() == Blast::FENCE_STATUS_COMPLETE) {
                mCommands[i]->fence->reset();
                return mCommands[i];
            }
        }
        CopyCommand* newCommand = new CopyCommand();
        newCommand->fence = mRenderer->mContext->createFence();
        newCommand->cmd = mRenderer->mCmdPool->allocBuf(false);
        mCommands.push_back(newCommand);
        return newCommand;
    }



}