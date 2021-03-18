#pragma once
#include <vector>

namespace Blast {
    class GfxFence;
    class GfxCommandBuffer;
}

namespace gear {
    class Renderer;

    struct CopySet {
        Blast::GfxFence* fence;
        Blast::GfxCommandBuffer* cmd;
    };

    class CopyEngine {
    public:
        CopyEngine(Renderer* renderer);
        ~CopyEngine();
        CopySet* getActiveSet();
    private:
        Renderer* mRenderer = nullptr;
        std::vector<CopySet*> mSets;
    };
}