#include "Renderer.h"
#include "RenderCache.h"
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
#include <Blast/Utility/ShaderCompiler.h>
#include <Blast/Utility/VulkanShaderCompiler.h>

namespace gear {
    Renderer::Renderer() {
        _shader_compiler = new blast::VulkanShaderCompiler();
        _context = new blast::VulkanContext();
        _queue = _context->GetQueue(blast::QUEUE_TYPE_GRAPHICS);

        blast::GfxCommandBufferPoolDesc cmd_pool_desc;
        cmd_pool_desc.queue = _queue;
        cmd_pool_desc.transient = false;
        _cmd_pool = _context->CreateCommandBufferPool(cmd_pool_desc);

        // Copy Command
        _copy_fence = _context->CreateFence();
        _copy_cmd = _cmd_pool->AllocBuffer(false);

        // 创建RootSignature
        blast::GfxRootSignatureDesc root_signature_desc;
        for (int i = 0; i < UBUFFER_BINDING_COUNT; ++i) {
            blast::GfxRegisterInfo register_info;
            register_info.set = 0;
            register_info.reg = i;
            register_info.size = 1;
            register_info.type = blast::RESOURCE_TYPE_UNIFORM_BUFFER;
            root_signature_desc.registers.push_back(register_info);
        }

        for (int i = 0; i < SAMPLER_BINDING_COUNT; ++i) {
            blast::GfxRegisterInfo register_info;
            register_info.set = 1;
            register_info.reg = i;
            register_info.size = 1;
            register_info.type = blast::RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER;
            root_signature_desc.registers.push_back(register_info);
        }
        _root_signature = _context->CreateRootSignature(root_signature_desc);

        // render cache
        _sampler_cache = new SamplerCache(this);
        _texture_view_cache = new TextureViewCache(this);
        _framebuffer_cache = new FramebufferCache(this);
        _graphics_pipeline_cache = new GraphicsPipelineCache(this);
        _descriptor_cache = new DescriptorCache(this);
    }

    Renderer::~Renderer() {
        _queue->WaitIdle();

        SAFE_DELETE(_sampler_cache);
        SAFE_DELETE(_texture_view_cache);
        SAFE_DELETE(_framebuffer_cache);
        SAFE_DELETE(_graphics_pipeline_cache);
        SAFE_DELETE(_descriptor_cache);

        // 执行所有清除任务
        for (auto task : _destroy_task_map) {
            task.second();
        }

        for (auto buffer :_stage_buffer_pool) {
            _context->DestroyBuffer(buffer);
        }

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

        _cmd_pool->DeleteBuffer(_copy_cmd);
        _context->DestroyFence(_copy_fence);

        _context->DestroyCommandBufferPool(_cmd_pool);
        _context->DestroySwapchain(_swapchain);
        _context->DestroySurface(_surface);
        _context->DestroyRootSignature(_root_signature);

        SAFE_DELETE(_context);
        SAFE_DELETE(_shader_compiler);
    }

    void Renderer::Resize(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) {
            return;
        }
        _queue->WaitIdle();

        // 处理之前绑定在渲染命令的资源
        for (uint32_t i = 0; i < _num_images; i++) {
            for (auto resource : _frames[i]->resources) {
                _using_resources[resource] = _using_resources[resource] - 1;
                if (_using_resources[resource] == 0) {
                    _using_resources.erase(resource);
                    for (auto iter = _destroy_task_map.begin(); iter!=_destroy_task_map.end();) {
                        if (iter->first == resource) {
                            iter->second();
                            iter = _destroy_task_map.erase(iter);
                        } else {
                            iter++;
                        }
                    }

                    // 回收暂缓缓存
                    if (find(_stage_buffer_pool.begin(), _stage_buffer_pool.end(), resource) != _stage_buffer_pool.end()) {
                        _usable_stage_buffer_list.push_back((blast::GfxBuffer*)resource);
                    }
                }
            }
            _frames[i]->resources.clear();
        }

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
        _copy_fence->WaitForComplete();
        // 处理之前绑定在Copy命令的资源
        for (auto resource : _copy_resources.resources) {
            _using_resources[resource] = _using_resources[resource] - 1;
            if (_using_resources[resource] == 0) {
                _using_resources.erase(resource);
                for (auto iter = _destroy_task_map.begin(); iter!=_destroy_task_map.end();) {
                    if (iter->first == resource) {
                        iter->second();
                        iter = _destroy_task_map.erase(iter);
                    } else {
                        iter++;
                    }
                }

                // 回收暂缓缓存
                if (find(_stage_buffer_pool.begin(), _stage_buffer_pool.end(), resource) != _stage_buffer_pool.end()) {
                    _usable_stage_buffer_list.push_back((blast::GfxBuffer*)resource);
                }
            }
        }
        _copy_resources.resources.clear();

        // 处理Copy任务
        _copy_cmd->Begin();
        while (_render_task_queue.size() > 0) {
            auto task = _render_task_queue.front();
            task(_copy_cmd);
            _render_task_queue.pop();
        }
        _copy_cmd->End();
        blast::GfxSubmitInfo submit_info;
        submit_info.num_cmd_bufs = 1;
        submit_info.cmd_bufs = &_copy_cmd;
        submit_info.signal_fence = _copy_fence;
        submit_info.num_wait_semaphores = 0;
        submit_info.wait_semaphores = nullptr;
        submit_info.num_signal_semaphores = 0;
        submit_info.signal_semaphores = nullptr;
        _queue->Submit(submit_info);

        _skip_frame = false;
        if (width == 0 || height == 0) {
            _skip_frame = true;
            return;
        }

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

        _context->AcquireNextImage(_swapchain, _image_acquired_semaphores[_frame_index], nullptr, &_swapchain_image_index);
        if (_swapchain_image_index == -1) {
            _skip_frame = true;
            return;
        }
        _render_complete_fences[_frame_index]->WaitForComplete();

        // 处理之前绑定在渲染命令的资源
        for (auto resource : _frames[_frame_index]->resources) {
            _using_resources[resource] = _using_resources[resource] - 1;
            if (_using_resources[resource] == 0) {
                _using_resources.erase(resource);
                for (auto iter = _destroy_task_map.begin(); iter!=_destroy_task_map.end();) {
                    if (iter->first == resource) {
                        iter->second();
                        iter = _destroy_task_map.erase(iter);
                    } else {
                        iter++;
                    }
                }

                // 回收暂缓缓存
                if (find(_stage_buffer_pool.begin(), _stage_buffer_pool.end(), resource) != _stage_buffer_pool.end()) {
                    _usable_stage_buffer_list.push_back((blast::GfxBuffer*)resource);
                }
            }
        }
        _frames[_frame_index]->resources.clear();

        blast::GfxTexture* color_rt = _swapchain->GetColorRenderTarget(_frame_index);
        blast::GfxTexture* depth_rt = _swapchain->GetDepthRenderTarget(_frame_index);

        _cmds[_frame_index]->Begin();

        // 设置标记
        _in_frame = true;

        {
            // 设置交换链RT为可写状态
            blast::GfxTextureBarrier barriers[2];
            barriers[0].texture = color_rt;
            barriers[0].new_state = blast::RESOURCE_STATE_RENDER_TARGET;
            barriers[1].texture = depth_rt;
            barriers[1].new_state = blast::RESOURCE_STATE_DEPTH_WRITE;
            _cmds[_frame_index]->SetBarrier(0, nullptr, 2, barriers);
        }

        // 在每一帧的开始清空当前fb
        FramebufferInfo screen_fb = {};
        screen_fb.is_screen_fb = true;
        screen_fb.clear_value.flags = blast::CLEAR_ALL;
        screen_fb.clear_value.color[0] = 0.2f;
        screen_fb.clear_value.color[1] = 0.3f;
        screen_fb.clear_value.color[2] = 0.8f;
        screen_fb.clear_value.color[3] = 1.0f;
        screen_fb.clear_value.depth = 1.0f;
        screen_fb.clear_value.stencil = 0;
        screen_fb.width = _frame_width;
        screen_fb.height = _frame_height;
        screen_fb.viewport[0] = 0.0f;
        screen_fb.viewport[1] = 0.0f;
        screen_fb.viewport[2] = _frame_width;
        screen_fb.viewport[3] = _frame_height;
        screen_fb.colors[0].texture = color_rt;
        screen_fb.colors[0].layer = 0;
        screen_fb.colors[0].level = 0;
        screen_fb.depth_stencil.texture = depth_rt;
        screen_fb.depth_stencil.layer = 0;
        screen_fb.depth_stencil.level = 0;
        BindFramebuffer(screen_fb);
        UnbindFramebuffer();
    }

    void Renderer::EndFrame() {
        if (_skip_frame) {
            return;
        }

        blast::GfxTexture* color_rt = _swapchain->GetColorRenderTarget(_frame_index);
        blast::GfxTexture* depth_rt = _swapchain->GetDepthRenderTarget(_frame_index);
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

        // 设置标记
        _in_frame = false;

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
        present_info.index = _swapchain_image_index;
        present_info.num_wait_semaphores = 1;
        present_info.wait_semaphores = &_render_complete_semaphores[_frame_index];
        _queue->Present(present_info);
        _frame_index = (_frame_index + 1) % _num_images;
    }

    blast::GfxTexture* Renderer::GetColor() {
        return _swapchain->GetColorRenderTarget(_frame_index);
    }

    blast::GfxTexture* Renderer::GetDepthStencil() {
        return _swapchain->GetDepthRenderTarget(_frame_index);
    }

    blast::GfxCommandBuffer* Renderer::GetCommandBuffer() {
        if (_in_frame) {
            return _cmds[_frame_index];
        }
        return _copy_cmd;
    }

    void Renderer::ExecRenderTask(std::function<void(blast::GfxCommandBuffer*)> task) {
        if (_in_frame) {
            task(_cmds[_frame_index]);
            return;
        }
        _render_task_queue.push(task);
    }

    void Renderer::UseResource(void* resource) {
        if (_in_frame) {
            auto iter = _frames[_frame_index]->resources.find(resource);
            if(iter == _frames[_frame_index]->resources.end()) {
                _frames[_frame_index]->resources.insert(resource);
                _using_resources[resource] = _using_resources[resource] + 1;
            }
        } else {
            auto iter = _copy_resources.resources.find(resource);
            if(iter == _copy_resources.resources.end()) {
                _copy_resources.resources.insert(resource);
                _using_resources[resource] = _using_resources[resource] + 1;
            }
        }
    }

    blast::GfxBuffer* Renderer::AllocStageBuffer(uint32_t size) {
        for (auto iter = _usable_stage_buffer_list.begin(); iter != _usable_stage_buffer_list.end(); iter++) {
            if ((*iter)->GetSize() >= size) {
                blast::GfxBuffer* buffer = *iter;
                _usable_stage_buffer_list.erase(iter);
                return buffer;
            }
        }
        // 没有合适的buffer，创建一个并存到pool里面
        blast::GfxBufferDesc buffer_desc;
        buffer_desc.size = size;
        buffer_desc.type = blast::RESOURCE_TYPE_RW_BUFFER;
        buffer_desc.usage = blast::RESOURCE_USAGE_CPU_TO_GPU;
        blast::GfxBuffer* staging_buffer = _context->CreateBuffer(buffer_desc);
        _stage_buffer_pool.push_back(staging_buffer);
        return staging_buffer;
    }

    void Renderer::Destroy(blast::GfxBuffer* buffer) {
        _destroy_task_map[buffer] = ([this, buffer]() {
            _context->DestroyBuffer(buffer);
        });
    }

    void Renderer::Destroy(blast::GfxTexture* texture) {
        _destroy_task_map[texture] = ([this, texture]() {
            _context->DestroyTexture(texture);
        });
    }

    void Renderer::Destroy(blast::GfxShader* shader) {
        _destroy_task_map[shader] = ([this, shader]() {
            _context->DestroyShader(shader);
        });
    }

    void Renderer::BindFramebuffer(const FramebufferInfo& info) {
        if (_skip_frame) {
            return;
        }

        _bind_fb_info = info;

        blast::GfxCommandBuffer* cmd = _cmds[_frame_index];

        uint32_t num_barriers = 0;
        blast::GfxTextureBarrier barriers[TARGET_COUNT + 1];

        blast::GfxClearValue clear_value;

        blast::GfxFramebufferDesc framebuffer_desc = {};
        framebuffer_desc.width = info.width;
        framebuffer_desc.height = info.height;
        framebuffer_desc.sample_count = info.sample_count;

        for (int i = 0; i < TARGET_COUNT; ++i) {
            if (info.colors[i].texture != nullptr) {
                barriers[num_barriers].texture = info.colors[i].texture;
                barriers[num_barriers].new_state = blast::RESOURCE_STATE_RENDER_TARGET;
                num_barriers++;

                blast::GfxTextureViewDesc texture_view_desc;
                texture_view_desc.texture = info.colors[i].texture;
                texture_view_desc.layer = info.colors[i].layer;
                texture_view_desc.level = info.colors[i].level;
                framebuffer_desc.colors[i] = _texture_view_cache->GetTextureView(texture_view_desc);
                framebuffer_desc.num_colors++;
            }
        }
        if (info.depth_stencil.texture != nullptr) {
            barriers[num_barriers].texture = info.depth_stencil.texture;
            barriers[num_barriers].new_state = blast::RESOURCE_STATE_DEPTH_WRITE;
            num_barriers++;

            blast::GfxTextureViewDesc texture_view_desc;
            texture_view_desc.texture = info.depth_stencil.texture;
            texture_view_desc.layer = info.depth_stencil.layer;
            texture_view_desc.level = info.depth_stencil.level;
            framebuffer_desc.depth_stencil = _texture_view_cache->GetTextureView(texture_view_desc);
            framebuffer_desc.has_depth_stencil = true;
        }

        // 屏幕的帧缓存不需要进行布局转换
        if (!info.is_screen_fb) {
            cmd->SetBarrier(0, nullptr, num_barriers, barriers);
        }

        blast::GfxFramebuffer* fb = _framebuffer_cache->GetFramebuffer(framebuffer_desc);
        cmd->BindFramebuffer(fb);
        cmd->ClearFramebuffer(fb, info.clear_value);
        cmd->SetViewport(info.viewport[0], info.viewport[1], info.viewport[2], info.viewport[3]);
        cmd->SetScissor(0, 0, info.width, info.height);
        _bind_fb = fb;
    }

    void Renderer::UnbindFramebuffer() {
        if (_skip_frame) {
            return;
        }

        blast::GfxCommandBuffer* cmd = _cmds[_frame_index];
        cmd->UnbindFramebuffer();

        uint32_t num_barriers = 0;
        blast::GfxTextureBarrier barriers[TARGET_COUNT + 1];

        for (int i = 0; i < TARGET_COUNT; ++i) {
            if (_bind_fb_info.colors[i].texture != nullptr) {
                barriers[num_barriers].texture = _bind_fb_info.colors[i].texture;
                barriers[num_barriers].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
                num_barriers++;
            }
        }
        if (_bind_fb_info.depth_stencil.texture != nullptr) {
            barriers[num_barriers].texture = _bind_fb_info.depth_stencil.texture;
            barriers[num_barriers].new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
            num_barriers++;
        }

        // 屏幕的帧缓存不需要进行布局转换
        if (!_bind_fb_info.is_screen_fb) {
            cmd->SetBarrier(0, nullptr, num_barriers, barriers);
        }
    }

    void Renderer::BindFrameUniformBuffer(blast::GfxBuffer* buffer, uint32_t size, uint32_t offset) {
        _bind_frame_uniform_buffer = buffer;
        _bind_frame_uniform_buffer_size = size;
        _bind_frame_uniform_buffer_offset = offset;
    }

    void Renderer::ExecuteDrawCall(const DrawCall& dc) {
        if (_skip_frame) {
            return;
        }

        blast::GfxCommandBuffer* cmd = _cmds[_frame_index];

        // 标记使用的外部资源
        UseResource(dc.bone_ub);
        UseResource(dc.renderable_ub);
        UseResource(dc.material_ub);
        UseResource(dc.vb);
        UseResource(dc.ib);
        UseResource(dc.vs);
        UseResource(dc.fs);
        UseResource(_bind_frame_uniform_buffer);

        DescriptorKey descriptor_key = {};
        // 绑定materialUB
        if (dc.material_ub) {
            descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].slot = 0;
            descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].uniform_buffer = dc.material_ub;
            descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].uniform_buffer_size = dc.material_ub_size;
            descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].uniform_buffer_offset = dc.material_ub_offset;
            descriptor_key.num_uniform_buffers++;
        }

        // 绑定frameUB
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].slot = 1;
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].uniform_buffer = _bind_frame_uniform_buffer;
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].uniform_buffer_size = _bind_frame_uniform_buffer_size;
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].uniform_buffer_offset = _bind_frame_uniform_buffer_offset;
        descriptor_key.num_uniform_buffers++;

        // 绑定renderableUB
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].slot = 2;
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].uniform_buffer = dc.renderable_ub;
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].uniform_buffer_size = dc.renderable_ub_size;
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].uniform_buffer_offset = dc.renderable_ub_offset;
        descriptor_key.num_uniform_buffers++;

        // 绑定samplers
        descriptor_key.num_samplers = dc.num_sampler_infos;
        for (uint32_t i = 0; i < dc.num_sampler_infos; ++i) {
            blast::GfxTextureViewDesc texture_view_desc;
            texture_view_desc.texture = dc.sampler_infos[i].texture;
            texture_view_desc.layer = dc.sampler_infos[i].layer;
            texture_view_desc.num_layers = dc.sampler_infos[i].num_layers;
            texture_view_desc.level = dc.sampler_infos[i].level;
            texture_view_desc.num_levels = dc.sampler_infos[i].num_levels;
            descriptor_key.sampler_descriptors[i].slot = dc.sampler_infos[i].slot;
            descriptor_key.sampler_descriptors[i].textures_view = _texture_view_cache->GetTextureView(texture_view_desc);
            descriptor_key.sampler_descriptors[i].sampler = _sampler_cache->GetSampler(dc.sampler_infos[i].sampler_desc);
        }

        DescriptorBundle descriptor_bundle = _descriptor_cache->GetDescriptor(descriptor_key);

        blast::GfxGraphicsPipelineDesc pipeline_desc = {};
        pipeline_desc.framebuffer = _bind_fb;
        pipeline_desc.root_signature = _root_signature;
        pipeline_desc.vertex_shader = dc.vs;
        pipeline_desc.pixel_shader = dc.fs;
        pipeline_desc.vertex_layout = dc.vertex_layout;

        pipeline_desc.depth_state.depth_test = true;
        pipeline_desc.depth_state.depth_write = true;

        if (dc.render_state.blending_mode == BlendingMode::BLENDING_MODE_OPAQUE) {
            pipeline_desc.blend_state.src_factors[0] = blast::BLEND_ONE;
            pipeline_desc.blend_state.dst_factors[0] = blast::BLEND_ZERO;
            pipeline_desc.blend_state.src_alpha_factors[0] = blast::BLEND_ONE;
            pipeline_desc.blend_state.dst_alpha_factors[0] = blast::BLEND_ZERO;
            pipeline_desc.blend_state.color_write_masks[0] = blast::COLOR_COMPONENT_ALL;
        } else if (dc.render_state.blending_mode == BlendingMode::BLENDING_MODE_TRANSPARENT) {
            // 半透材质不应该写入深度缓存
            pipeline_desc.depth_state.depth_write = false;

            // 预乘使用的混合方程
            pipeline_desc.blend_state.src_factors[0] = blast::BLEND_ONE;
            pipeline_desc.blend_state.dst_factors[0] = blast::BLEND_ONE_MINUS_SRC_ALPHA;
            pipeline_desc.blend_state.src_alpha_factors[0] = blast::BLEND_ONE;
            pipeline_desc.blend_state.dst_alpha_factors[0] = blast::BLEND_ONE_MINUS_SRC_ALPHA;
            pipeline_desc.blend_state.color_write_masks[0] = blast::COLOR_COMPONENT_ALL;

        } else if (dc.render_state.blending_mode == BlendingMode::BLENDING_MODE_MASKED) {
            pipeline_desc.blend_state.src_factors[0] = blast::BLEND_ONE;
            pipeline_desc.blend_state.dst_factors[0] = blast::BLEND_ZERO;
            pipeline_desc.blend_state.src_alpha_factors[0] = blast::BLEND_ZERO;
            pipeline_desc.blend_state.dst_alpha_factors[0] = blast::BLEND_ONE;
            pipeline_desc.blend_state.color_write_masks[0] = blast::COLOR_COMPONENT_ALL;
        }

        pipeline_desc.rasterizer_state.primitive_topo = dc.topo;
        pipeline_desc.rasterizer_state.front_face = blast::FRONT_FACE_CCW;
        pipeline_desc.rasterizer_state.cull_mode = dc.render_state.cull_mode;

        blast::GfxGraphicsPipeline* pipeline = _graphics_pipeline_cache->GetGraphicsPipeline(pipeline_desc);
        cmd->BindGraphicsPipeline(pipeline);
        cmd->BindDescriptorSets(_root_signature, 2, descriptor_bundle.handles);

        if (dc.ib != nullptr) {
            cmd->BindVertexBuffer(dc.vb, 0);
            cmd->BindIndexBuffer(dc.ib, 0, dc.ib_type);
            cmd->DrawIndexed(dc.ib_count, 1, dc.ib_offset, 0, 0);
        } else {
            cmd->BindVertexBuffer(dc.vb, 0);
            cmd->Draw(dc.vb_count, 1, dc.vb_offset, 0);
        }
    }

    void Renderer::ExecuteDebugDrawCall(const DrawCall& dc) {
        if (_skip_frame) {
            return;
        }

        blast::GfxCommandBuffer* cmd = _cmds[_frame_index];

        // 标记使用的外部资源
        UseResource(dc.vb);
        UseResource(dc.vs);
        UseResource(dc.fs);
        UseResource(dc.renderable_ub);
        UseResource(_bind_frame_uniform_buffer);

        DescriptorKey descriptor_key = {};

        // 绑定frameUB
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].slot = 1;
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].uniform_buffer = _bind_frame_uniform_buffer;
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].uniform_buffer_size = _bind_frame_uniform_buffer_size;
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].uniform_buffer_offset = _bind_frame_uniform_buffer_offset;
        descriptor_key.num_uniform_buffers++;

        // 绑定renderableUB
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].slot = 2;
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].uniform_buffer = dc.renderable_ub;
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].uniform_buffer_size = dc.renderable_ub_size;
        descriptor_key.uniform_descriptors[descriptor_key.num_uniform_buffers].uniform_buffer_offset = dc.renderable_ub_offset;
        descriptor_key.num_uniform_buffers++;

        DescriptorBundle descriptor_bundle = _descriptor_cache->GetDescriptor(descriptor_key);

        blast::GfxGraphicsPipelineDesc pipeline_desc = {};
        pipeline_desc.framebuffer = _bind_fb;
        pipeline_desc.root_signature = _root_signature;
        pipeline_desc.vertex_shader = dc.vs;
        pipeline_desc.pixel_shader = dc.fs;
        pipeline_desc.vertex_layout = dc.vertex_layout;

        pipeline_desc.depth_state.depth_test = false;
        pipeline_desc.depth_state.depth_write = false;

        pipeline_desc.blend_state.src_factors[0] = blast::BLEND_ONE;
        pipeline_desc.blend_state.dst_factors[0] = blast::BLEND_ZERO;
        pipeline_desc.blend_state.src_alpha_factors[0] = blast::BLEND_ONE;
        pipeline_desc.blend_state.dst_alpha_factors[0] = blast::BLEND_ZERO;
        pipeline_desc.blend_state.color_write_masks[0] = blast::COLOR_COMPONENT_ALL;

        pipeline_desc.rasterizer_state.primitive_topo = dc.topo;
        pipeline_desc.rasterizer_state.cull_mode = dc.render_state.cull_mode;

        blast::GfxGraphicsPipeline* pipeline = _graphics_pipeline_cache->GetGraphicsPipeline(pipeline_desc);
        cmd->BindGraphicsPipeline(pipeline);
        cmd->BindDescriptorSets(_root_signature, 2, descriptor_bundle.handles);
        cmd->BindVertexBuffer(dc.vb, 0);
        cmd->Draw(dc.vb_count, 1, dc.vb_offset, 0);
    }
}