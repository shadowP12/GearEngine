#pragma once
#include <map>
#include <vector>
#include <unordered_set>

namespace Blast {
    class GfxFence;
    class GfxBuffer;
    class GfxCommandBuffer;
}

namespace gear {
    class Renderer;

    struct CopyCommand {
        Blast::GfxFence* fence;
        Blast::GfxCommandBuffer* cmd;
    };

    // TODO: 支持stage buffer缓存
    class CopyEngine {
    public:
        CopyEngine(Renderer* renderer);

        ~CopyEngine();

        CopyCommand* getActiveCommand();

    private:
        Renderer* mRenderer = nullptr;
        std::vector<CopyCommand*> mCommands;
    };
}