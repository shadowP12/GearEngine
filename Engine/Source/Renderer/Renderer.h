#pragma once
#include "Core/GearDefine.h"
#include <set>
#include <map>
#include <queue>
#include <functional>

namespace blast {
    class ShaderCompiler;
    class GfxContext;
    class GfxSurface;
    class GfxSwapchain;
    class GfxRenderPass;
    class GfxFramebuffer;
    class GfxQueue;
    class GfxFence;
    class GfxSemaphore;
    class GfxSemaphore;
    class GfxCommandBufferPool;
    class GfxCommandBuffer;
    class GfxRootSignature;
    class GfxGraphicsPipeline;
    class GfxSampler;
}

namespace gear {
    class Renderer {
    public:
        struct Frame {
            std::set<void*> resources;
        };

    public:
        Renderer();

        ~Renderer();

        void Resize(uint32_t width, uint32_t height);

        void BeginFrame(void* window, uint32_t width, uint32_t height);

        void EndFrame();

        blast::GfxContext* GetContext() { return _context; }

        blast::GfxQueue* GetQueue() { return _queue; }

        void AcquireResource(void* resource);

    private:
        blast::GfxContext* _context = nullptr;
        blast::GfxSurface* _surface = nullptr;
        blast::GfxSwapchain* _swapchain = nullptr;
        blast::GfxQueue* _queue = nullptr;
        blast::GfxFence** _render_complete_fences = nullptr;
        blast::GfxSemaphore** _image_acquired_semaphores = nullptr;
        blast::GfxSemaphore** _render_complete_semaphores = nullptr;
        blast::GfxCommandBufferPool* _cmd_pool = nullptr;
        blast::GfxCommandBuffer** _cmds = nullptr;
        uint32_t _num_images = 0;
        uint32_t _frame_index = 0;
        uint32_t _frame_width = 0;
        uint32_t _frame_height = 0;
        Frame** _frames = nullptr;
        // 外部window
        void* _window = nullptr;
        // 记录gpu正在使用的资源
        std::map<void*, uint32_t> _using_resources;
        // 渲染器的任务队列
        std::queue<std::function<void()>> _render_task_queue;
        // 资源销毁的回调队列
        std::queue<std::function<void()>> _destroy_task_queue;
    };
}