#include "View.h"
#include "GearEngine.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Window/BaseWindow.h"

#include <Blast/Gfx/GfxDevice.h>

namespace gear {
    View::View() {

    }

    View::~View() {
        if (renderpass) {
            gEngine.GetDevice()->DestroyRenderPass(renderpass);
        }
        if (main_rt) {
            gEngine.GetDevice()->DestroyTexture(main_rt);
        }
        if (main_resolve_rt) {
            gEngine.GetDevice()->DestroyTexture(main_resolve_rt);
        }
        if (depth_rt) {
            gEngine.GetDevice()->DestroyTexture(depth_rt);
        }
        if (depth_resolve_rt) {
            gEngine.GetDevice()->DestroyTexture(depth_resolve_rt);
        }
    }

    void View::SetSize(float w, float h) {
        size.x = w;
        size.y = h;
    }

    void View::SetViewport(float x, float y, float w, float h) {
        viewport.x = x;
        viewport.y = y;
        viewport.z = w;
        viewport.w = h;
    }

    void View::SetSampleCount(blast::SampleCount sm) {
        sample_count = sm;
    }

    bool View::ResizeBuffers() {
        if (size.x == 0.0f || size.y == 0.0f) {
            return false;
        }

        bool should_resize = false;
        if (old_size.x != size.x || old_size.y != size.y) {
            old_size.x = size.x;
            old_size.y = size.y;
            should_resize = true;
        }

        if (sample_count != old_sample_count) {
            old_sample_count = sample_count;
            should_resize = true;
        }

        blast::GfxDevice* device = gEngine.GetDevice();
        if (should_resize) {
            blast::GfxTextureDesc texture_desc = {};
            texture_desc.width = size.x;
            texture_desc.height = size.y;
            texture_desc.sample_count = sample_count;
            texture_desc.format = blast::FORMAT_R8G8B8A8_UNORM;
            texture_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
            texture_desc.res_usage = blast::RESOURCE_USAGE_SHADER_RESOURCE | blast::RESOURCE_USAGE_RENDER_TARGET;
            texture_desc.clear.color[0] = 0.0f;
            texture_desc.clear.color[1] = 0.0f;
            texture_desc.clear.color[2] = 0.0f;
            texture_desc.clear.color[3] = 0.0f;
            // 默认深度值为1
            texture_desc.clear.depthstencil.depth = 1.0f;
            main_rt = device->CreateTexture(texture_desc);

            texture_desc.format = blast::FORMAT_D24_UNORM_S8_UINT;
            texture_desc.res_usage = blast::RESOURCE_USAGE_SHADER_RESOURCE | blast::RESOURCE_USAGE_DEPTH_STENCIL;
            depth_rt = device->CreateTexture(texture_desc);

            // 当进行多重采样的时候需要额外创建ResolveRT
            if (sample_count > 1) {
                texture_desc.sample_count = blast::SAMPLE_COUNT_1;
                texture_desc.format = blast::FORMAT_R8G8B8A8_UNORM;
                texture_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
                texture_desc.res_usage = blast::RESOURCE_USAGE_SHADER_RESOURCE | blast::RESOURCE_USAGE_RENDER_TARGET;
                main_resolve_rt = device->CreateTexture(texture_desc);
            }

            blast::GfxRenderPassDesc renderpass_desc = {};
            renderpass_desc.attachments.push_back(blast::RenderPassAttachment::RenderTarget(main_rt, -1, blast::LOAD_CLEAR));
            renderpass_desc.attachments.push_back(
                    blast::RenderPassAttachment::DepthStencil(
                            depth_rt,
                            -1,
                            blast::LOAD_CLEAR,
                            blast::STORE_STORE
                    )
            );
            if (sample_count > 1) {
                renderpass_desc.attachments.push_back(blast::RenderPassAttachment::Resolve(main_resolve_rt));
            }
            renderpass = device->CreateRenderPass(renderpass_desc);
        }

        return true;
    }

    bool View::Prepare(blast::GfxCommandBuffer* cmd) {
        if (!ResizeBuffers()) {
            return false;
        }

        return true;
    }


}