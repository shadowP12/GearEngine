#include "Renderer.h"
#include "Utility/FileSystem.h"
#include <Blast/Gfx/GfxContext.h>
#include <Blast/Gfx/GfxBuffer.h>
#include <Blast/Gfx/GfxTexture.h>
#include <Blast/Gfx/GfxSampler.h>
#include <Blast/Gfx/GfxSwapchain.h>
#include <Blast/Gfx/GfxCommandBuffer.h>
#include <Blast/Gfx/GfxShader.h>
#include <Blast/Gfx/GfxPipeline.h>
#include <Blast/Gfx/Vulkan/VulkanContext.h>

namespace gear {
    Renderer::Renderer() {
        _context = new blast::VulkanContext();
        _queue = _context->GetQueue(blast::QUEUE_TYPE_GRAPHICS);

        blast::GfxCommandBufferPoolDesc cmd_pool_desc;
        cmd_pool_desc.queue = _queue;
        cmd_pool_desc.transient = false;
        _cmd_pool = _context->CreateCommandBufferPool(cmd_pool_desc);
    }

    Renderer::~Renderer() {
        _queue->WaitIdle();

        for (uint32_t i = 0; i < _num_images; ++i) {
            _cmd_pool->DeleteBuffer(_cmds[i]);
            _context->DestroyFence(_render_complete_fences[i]);
            _context->DestroySemaphore(_image_acquired_semaphores[i]);
            _context->DestroySemaphore(_render_complete_semaphores[i]);
            SAFE_DELETE(_frames[i]);
        }
        SAFE_DELETE_ARRAY(_cmds);
        SAFE_DELETE_ARRAY(_render_complete_fences);
        SAFE_DELETE_ARRAY(_image_acquired_semaphores);
        SAFE_DELETE_ARRAY(_render_complete_semaphores);
        SAFE_DELETE_ARRAY(_frames);

        _context->DestroyCommandBufferPool(_cmd_pool);
        _context->DestroySwapchain(_swapchain);
        _context->DestroySurface(_surface);

        SAFE_DELETE(_context);
    }

    void Renderer::Resize(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) {
            return;
        }
        _queue->WaitIdle();

        blast::GfxSwapchain* old_swapchain = _swapchain;
        blast::GfxSwapchainDesc swapchain_desc;
        swapchain_desc.width = width;
        swapchain_desc.height = height;
        swapchain_desc.surface = _surface;
        swapchain_desc.old_swapchain = old_swapchain;
        _swapchain = _context->CreateSwapchain(swapchain_desc);
        _context->DestroySwapchain(old_swapchain);

        for (uint32_t i = 0; i < _num_images; ++i) {
            _cmd_pool->DeleteBuffer(_cmds[i]);
            _context->DestroyFence(_render_complete_fences[i]);
            _context->DestroySemaphore(_image_acquired_semaphores[i]);
            _context->DestroySemaphore(_render_complete_semaphores[i]);
            SAFE_DELETE(_frames[i]);
        }
        SAFE_DELETE_ARRAY(_cmds);
        SAFE_DELETE_ARRAY(_render_complete_fences);
        SAFE_DELETE_ARRAY(_image_acquired_semaphores);
        SAFE_DELETE_ARRAY(_render_complete_semaphores);
        SAFE_DELETE_ARRAY(_frames);

        _num_images = _swapchain->GetImageCount();
        _frames = new Frame*[_num_images];
        _render_complete_fences = new blast::GfxFence*[_num_images];
        _image_acquired_semaphores = new blast::GfxSemaphore*[_num_images];
        _render_complete_semaphores = new blast::GfxSemaphore*[_num_images];
        _cmds = new blast::GfxCommandBuffer*[_num_images];
        for (uint32_t i = 0; i < _num_images; ++i) {
            _frames[i] = new Frame();
            _render_complete_fences[i] = _context->CreateFence();
            _image_acquired_semaphores[i] = _context->CreateSemaphore();
            _render_complete_semaphores[i] = _context->CreateSemaphore();

            // 创建新的命令缓存并设置交换链RT为显示状态
            blast::GfxTexture* color_rt = _swapchain->GetColorRenderTarget(i);
            blast::GfxTexture* depth_rt = _swapchain->GetDepthRenderTarget(i);
            _cmds[i] = _cmd_pool->AllocBuffer(false);
            _cmds[i]->Begin();
            blast::GfxTextureBarrier barriers[2];
            barriers[0].texture = color_rt;
            barriers[0].new_state = blast::RESOURCE_STATE_PRESENT;
            barriers[1].texture = depth_rt;
            barriers[1].new_state = blast::RESOURCE_STATE_DEPTH_WRITE;
            _cmds[i]->SetBarrier(0, nullptr, 2, barriers);
            _cmds[i]->End();
        }

        blast::GfxSubmitInfo submit_info;
        submit_info.num_cmd_bufs = _num_images;
        submit_info.cmd_bufs = _cmds;
        submit_info.signal_fence = nullptr;
        submit_info.num_wait_semaphores = 0;
        submit_info.wait_semaphores = nullptr;
        submit_info.num_signal_semaphores = 0;
        submit_info.signal_semaphores = nullptr;
        _queue->Submit(submit_info);
        _queue->WaitIdle();
    }

    void Renderer::BeginFrame(void* window, uint32_t width, uint32_t height) {
        if (_window != window || _frame_width != width || _frame_height != height) {
            if (_window != window) {
                _context->DestroySurface(_surface);
                blast::GfxSurfaceDesc surface_desc;
                surface_desc.origin_surface = window;
                _surface = _context->CreateSurface(surface_desc);
            }
            _window = window;
            _frame_width = width;
            _frame_height = height;
            Resize(_frame_width, _frame_height);
        }

        uint32_t swapchain_image_index;
        _context->AcquireNextImage(_swapchain, _image_acquired_semaphores[_frame_index], nullptr, &swapchain_image_index);
        if (swapchain_image_index == -1) {
            return;
        }
        _render_complete_fences[_frame_index]->WaitForComplete();

        // 处理资源销毁任务


        blast::GfxTexture* color_rt = _swapchain->GetColorRenderTarget(_frame_index);
        blast::GfxTexture* depth_rt = _swapchain->GetDepthRenderTarget(_frame_index);

        _cmds[_frame_index]->Begin();
        // 处理渲染任务

        {
            // 设置交换链RT为可写状态
            blast::GfxTextureBarrier barriers[2];
            barriers[0].texture = color_rt;
            barriers[0].new_state = blast::RESOURCE_STATE_RENDER_TARGET;
            barriers[1].texture = depth_rt;
            barriers[1].new_state = blast::RESOURCE_STATE_DEPTH_WRITE;
            _cmds[_frame_index]->SetBarrier(0, nullptr, 2, barriers);
        }

        {
            // 设置交换链RT为显示状态
            blast::GfxTextureBarrier barriers[2];
            barriers[0].texture = color_rt;
            barriers[0].new_state = blast::RESOURCE_STATE_PRESENT ;
            barriers[1].texture = depth_rt;
            barriers[1].new_state = blast::RESOURCE_STATE_DEPTH_WRITE ;
            _cmds[_frame_index]->SetBarrier(0, nullptr, 2, barriers);
        }
        _cmds[_frame_index]->End();

        blast::GfxSubmitInfo submit_info;
        submit_info.num_cmd_bufs = 1;
        submit_info.cmd_bufs = &_cmds[_frame_index];
        submit_info.signal_fence = _render_complete_fences[_frame_index];
        submit_info.num_wait_semaphores = 1;
        submit_info.wait_semaphores = &_image_acquired_semaphores[_frame_index];
        submit_info.num_signal_semaphores = 1;
        submit_info.signal_semaphores = &_render_complete_semaphores[_frame_index];
        _queue->Submit(submit_info);

        blast::GfxPresentInfo present_info;
        present_info.swapchain = _swapchain;
        present_info.index = swapchain_image_index;
        present_info.num_wait_semaphores = 1;
        present_info.wait_semaphores = &_render_complete_semaphores[_frame_index];
        _queue->Present(present_info);
        _frame_index = (_frame_index + 1) % _num_images;
    }

    void Renderer::EndFrame() {
    }
}