#pragma once
#include <map>
#include <queue>
#include <vector>
#include <functional>
#include <unordered_set>

namespace Blast {
    class GfxFence;
    class GfxBuffer;
    class GfxTexture;
    class GfxCommandBuffer;
}

namespace gear {
    class Renderer;

    struct CopyCommand {
        Blast::GfxFence* fence;
        Blast::GfxCommandBuffer* cmd;
        std::vector<std::function<void()>> callbacks;
        bool used = false;
    };

    // TODO: 支持stage buffer缓存
    class CopyEngine {
    public:
        CopyEngine(Renderer* renderer);

        ~CopyEngine();

        void update();

        CopyCommand* getActiveCommand();

        Blast::GfxBuffer* acquireStage(uint32_t size);

        void releaseStage(Blast::GfxBuffer* stage);

        void destroy(Blast::GfxBuffer* buffer);

        void destroy(Blast::GfxTexture* texture);

    private:
        uint32_t mStageBufferSize = 0;
        Renderer* mRenderer = nullptr;
        std::vector<CopyCommand*> mCommands;
        std::queue<Blast::GfxBuffer*> mDestroyBuffer;
        std::queue<Blast::GfxTexture*> mDestroyTexture;
    };
}