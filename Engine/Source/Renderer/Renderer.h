#pragma once
#include "Core/GearDefine.h"
#include "RenderData.h"
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
    class GfxBuffer;
    class GfxTexture;
    class GfxShader;
}

namespace gear {
    class SamplerCache;
    class TextureViewCache;
    class FramebufferCache;
    class GraphicsPipelineCache;
    class DescriptorCache;
    class Renderer {
    public:
        struct Frame {
            std::set<void*> resources;
        };

    public:
        Renderer();

        ~Renderer();

        void Resize(uint32_t width, uint32_t height);

        void Render(void* window, uint32_t width, uint32_t height);

        blast::GfxContext* GetContext() { return _context; }

        blast::GfxQueue* GetQueue() { return _queue; }

        blast::GfxRootSignature* GetRootSignature() { return _root_signature; }

        blast::ShaderCompiler* GetShaderCompiler() { return _shader_compiler; }

        blast::GfxTexture* GetColor();

        blast::GfxTexture* GetDepthStencil();

        uint32_t GetWidth() { return _frame_width; }

        uint32_t GetHeight() { return _frame_height; }

        // 渲染器执行display任务
        void EnqueueDisplayTask(std::function<void()>);

        // 渲染器执行upload任务
        void EnqueueUploadTask(std::function<void()>);

        blast::GfxBuffer* AllocStageBuffer(uint32_t size);

        void Destroy(blast::GfxBuffer*);

        void Destroy(blast::GfxTexture*);

        void Destroy(blast::GfxShader*);

        void SetBarrier(const blast::GfxBufferBarrier&);

        void SetBarrier(const blast::GfxTextureBarrier&);

        void CopyToBuffer(const blast::GfxCopyToBufferRange&);

        void CopyToImage(const blast::GfxCopyToImageRange&);

        void BindFramebuffer(const FramebufferInfo& info);

        void UnbindFramebuffer();

        void BindVertexShader(blast::GfxShader* vs);

        void BindFragmentShader(blast::GfxShader* fs);

        void ResetUniformBufferSlot();

        void BindFrameUniformBuffer(blast::GfxBuffer* buffer, uint32_t size, uint32_t offset);

        void BindRenderableUniformBuffer(blast::GfxBuffer* buffer, uint32_t size, uint32_t offset);

        void BindMaterialUniformBuffer(blast::GfxBuffer* buffer, uint32_t size, uint32_t offset);

        void ResetSamplerSlot();

        void BindSampler(const SamplerInfo& info);

        void SetDepthState(bool enable_warte, bool enable_test = true);

        void SetBlendingMode(const BlendingMode& blending_mode);

        void SetPrimitiveTopo(const blast::PrimitiveTopology&);

        void SetFrontFace(const blast::FrontFace& front_face = blast::FRONT_FACE_CCW);

        void SetCullMode(const blast::CullMode&);

        void BindVertexBuffer(blast::GfxBuffer* buffer, const blast::GfxVertexLayout& layout, uint32_t size, uint32_t offset);

        void BindIndexBuffer(blast::GfxBuffer* buffer, const blast::IndexType& type, uint32_t size, uint32_t offset);

        void Draw(uint32_t count, uint32_t offset);

        void DrawIndexed(uint32_t count, uint32_t offset);

    private:
        blast::GfxCommandBuffer* GetCommandBuffer();

        void UseResource(void* resource);

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
        blast::GfxRootSignature* _root_signature = nullptr;
        blast::GfxFence* _copy_fence = nullptr;
        blast::GfxCommandBuffer* _upload_cmd = nullptr;
        bool _in_frame = false;
        bool _skip_frame = false;
        uint32_t  _swapchain_image_index;
        uint32_t _num_images = 0;
        uint32_t _frame_index = 0;
        uint32_t _frame_width = 0;
        uint32_t _frame_height = 0;
        // 外部window
        void* _window = nullptr;
        // shader编译器
        blast::ShaderCompiler* _shader_compiler = nullptr;
        // gpu对象缓存
        SamplerCache* _sampler_cache;
        TextureViewCache* _texture_view_cache;
        FramebufferCache* _framebuffer_cache;
        GraphicsPipelineCache* _graphics_pipeline_cache;
        DescriptorCache* _descriptor_cache;
        // 暂存缓存池
        std::vector<blast::GfxBuffer*> _stage_buffer_pool;
        std::vector<blast::GfxBuffer*> _usable_stage_buffer_list;
        // 记录gpu正在使用的资源
        Frame** _frames = nullptr;
        Frame _copy_resources;
        std::map<void*, uint32_t> _using_resources;
        // 渲染器的任务队列
        std::queue<std::function<void()>> _display_task_queue;
        std::queue<std::function<void()>> _upload_task_queue;
        // 资源销毁的回调队列
        std::map<void*, std::function<void()>> _destroy_task_map;
        // 渲染器绑定的资源
        FramebufferInfo _bind_fb_info;
        blast::GfxFramebuffer* _bind_fb = nullptr;
        DescriptorKey _descriptor_key;
        blast::GfxGraphicsPipelineDesc _graphics_pipeline_key;
    };
}