#pragma once
#include "Core/GearDefine.h"

#include <vector>

namespace blast {
    class GfxSwapChain;
}

namespace gear {
    class View;

    class BaseWindow {
    public:
        BaseWindow();

        ~BaseWindow();

        uint32_t GetWidth() { return width; }

        uint32_t GetHeight() { return height; }

    protected:
        blast::GfxSwapChain* GetSwapChain();

    protected:
        friend class View;
        friend class Renderer;
        friend class BaseApplication;
        void* window_ptr = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        blast::GfxSwapChain* swapchain = nullptr;
    };
}