#include "CopyEngine.h"
#include "Renderer.h"
#include "Utility/Log.h"
#include <Blast/Gfx/GfxBuffer.h>
#include <Blast/Gfx/GfxTexture.h>
#include <Blast/Gfx/GfxContext.h>
#include <Blast/Gfx/GfxCommandBuffer.h>

namespace gear {
    CopyEngine::CopyEngine(Renderer* renderer) {
        mRenderer = renderer;
    }

    CopyEngine::~CopyEngine() {
        for (int i = 0; i < mCommands.size(); ++i) {
            SAFE_DELETE(mCommands[i]->fence);
            SAFE_DELETE(mCommands[i]->cmd);
            // 触发回调
            for (int j = 0; j < mCommands[i]->callbacks.size(); ++j) {
                mCommands[i]->callbacks[j]();
            }
            mCommands[i]->callbacks.clear();
            SAFE_DELETE(mCommands[i]);
        }
        mCommands.clear();

        while (mDestroyBuffer.size() > 0) {
            Blast::GfxBuffer* buffer = mDestroyBuffer.front();
            SAFE_DELETE(buffer);
            mDestroyBuffer.pop();
        }

        while (mDestroyTexture.size() > 0) {
            Blast::GfxTexture* texture = mDestroyTexture.front();
            SAFE_DELETE(texture);
            mDestroyTexture.pop();
        }
    }

    void CopyEngine::update() {
        for (int i = 0; i < mCommands.size(); ++i) {
            if (mCommands[i]->used && mCommands[i]->fence->getFenceStatus() == Blast::FENCE_STATUS_COMPLETE) {
                mCommands[i]->used = false;
                // 触发回调
                for (int j = 0; j < mCommands[i]->callbacks.size(); ++j) {
                    mCommands[i]->callbacks[j]();
                }
                mCommands[i]->callbacks.clear();
            }
        }

        // TODO: 后续根据cmd引用来决定释放时机
        if (mDestroyBuffer.size() > 0 || mDestroyTexture.size() > 0) {
            // 强行等待所有命令执行完毕很影响性能，后续要采用另外方案
            mRenderer->terminate();

            while (mDestroyBuffer.size() > 0) {
                Blast::GfxBuffer* buffer = mDestroyBuffer.front();
                SAFE_DELETE(buffer);
                mDestroyBuffer.pop();
            }

            while (mDestroyTexture.size() > 0) {
                Blast::GfxTexture* texture = mDestroyTexture.front();
                SAFE_DELETE(texture);
                mDestroyTexture.pop();
            }
        }
    }

    CopyCommand* CopyEngine::getActiveCommand() {
        for (int i = 0; i < mCommands.size(); ++i) {
            if (!mCommands[i]->used && mCommands[i]->fence->getFenceStatus() == Blast::FENCE_STATUS_NOTSUBMITTED) {
                mCommands[i]->used = true;
                return mCommands[i];
            }
        }
        CopyCommand* newCommand = new CopyCommand();
        // 创建完fence后需要手动重置fence状态
        newCommand->fence = mRenderer->mContext->createFence();
        newCommand->cmd = mRenderer->mCmdPool->allocBuf(false);
        newCommand->used = true;
        mCommands.push_back(newCommand);
        return newCommand;
    }

    Blast::GfxBuffer* CopyEngine::acquireStage(uint32_t size) {
        mStageBufferSize++;
        return nullptr;
    }

    void CopyEngine::releaseStage(Blast::GfxBuffer* stage) {
        mStageBufferSize--;
    }

    void CopyEngine::destroy(Blast::GfxBuffer* buffer) {
        mDestroyBuffer.push(buffer);
    }

    void CopyEngine::destroy(Blast::GfxTexture* texture) {
        mDestroyTexture.push(texture);
    }

}