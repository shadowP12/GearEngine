#include "View.h"
#include "GearEngine.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Window/BaseWindow.h"

#include <Blast/Gfx/GfxDevice.h>

namespace gear {
    View::View() {
        debug_lines.resize(MAX_DEBUG_LINES * 14);
    }

    View::~View() {
        blast::GfxDevice* device = gEngine.GetDevice();
        if (renderpass) {
            device->DestroyRenderPass(renderpass);
        }
        if (main_rt) {
            device->DestroyTexture(main_rt);
        }
        if (main_resolve_rt) {
            device->DestroyTexture(main_resolve_rt);
        }
        if (depth_rt) {
            device->DestroyTexture(depth_rt);
        }
        if (depth_resolve_rt) {
            device->DestroyTexture(depth_resolve_rt);
        }
        if (postprocess_rt0) {
            device->DestroyTexture(postprocess_rt0);
        }
        if (postprocess_rt1) {
            device->DestroyTexture(postprocess_rt1);
        }
        if (fxaa_renderpass0) {
            device->DestroyRenderPass(fxaa_renderpass0);
        }
        if (fxaa_renderpass1) {
            device->DestroyRenderPass(fxaa_renderpass1);
        }

		if (atmosphere_raymarching_renderpass0) {
			device->DestroyRenderPass(atmosphere_raymarching_renderpass0);
		}
		if (atmosphere_raymarching_renderpass1) {
			device->DestroyRenderPass(atmosphere_raymarching_renderpass1);
		}

        if (debug_renderpass0) {
            device->DestroyRenderPass(debug_renderpass0);
        }
        if (debug_renderpass1) {
            device->DestroyRenderPass(debug_renderpass1);
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

            {
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

            {
                if (postprocess_rt0) {
                    device->DestroyTexture(postprocess_rt0);
                }
                if (postprocess_rt1) {
                    device->DestroyTexture(postprocess_rt1);
                }
                texture_desc.sample_count = blast::SAMPLE_COUNT_1;
                texture_desc.format = blast::FORMAT_R8G8B8A8_UNORM;
                texture_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
                texture_desc.res_usage = blast::RESOURCE_USAGE_SHADER_RESOURCE | blast::RESOURCE_USAGE_RENDER_TARGET;
                postprocess_rt0 = device->CreateTexture(texture_desc);
                postprocess_rt1 = device->CreateTexture(texture_desc);
            }

            {
                if (fxaa_renderpass0) {
                    device->DestroyRenderPass(fxaa_renderpass0);
                }
				if (debug_renderpass0) {
					device->DestroyRenderPass(debug_renderpass0);
				}
                blast::GfxRenderPassDesc renderpass_desc = {};
                renderpass_desc.attachments.push_back(blast::RenderPassAttachment::RenderTarget(postprocess_rt0, -1, blast::LOAD_CLEAR));
                fxaa_renderpass0 = device->CreateRenderPass(renderpass_desc);
                debug_renderpass0 = device->CreateRenderPass(renderpass_desc);
            }
            {
                if (fxaa_renderpass1) {
                    device->DestroyRenderPass(fxaa_renderpass1);
                }
				if (debug_renderpass1) {
					device->DestroyRenderPass(debug_renderpass1);
				}
                blast::GfxRenderPassDesc renderpass_desc = {};
                renderpass_desc.attachments.push_back(blast::RenderPassAttachment::RenderTarget(postprocess_rt1, -1, blast::LOAD_CLEAR));
                fxaa_renderpass1 = device->CreateRenderPass(renderpass_desc);
                debug_renderpass1 = device->CreateRenderPass(renderpass_desc);
            }

			{
				if (atmosphere_raymarching_renderpass0) {
					device->DestroyRenderPass(atmosphere_raymarching_renderpass0);
				}
				if (atmosphere_raymarching_renderpass1) {
					device->DestroyRenderPass(atmosphere_raymarching_renderpass1);
				}

				blast::GfxRenderPassDesc renderpass_desc = {};
				renderpass_desc.attachments.push_back(blast::RenderPassAttachment::RenderTarget(postprocess_rt0, -1, blast::LOAD_LOAD));
				atmosphere_raymarching_renderpass0 = device->CreateRenderPass(renderpass_desc);

				renderpass_desc.attachments.clear();
				renderpass_desc.attachments.push_back(blast::RenderPassAttachment::RenderTarget(postprocess_rt1, -1, blast::LOAD_LOAD));
				atmosphere_raymarching_renderpass1 = device->CreateRenderPass(renderpass_desc);
			}
        }

        return true;
    }

    bool View::Prepare(blast::GfxCommandBuffer* cmd) {
        if (!ResizeBuffers()) {
            return false;
        }

        return true;
    }

    void View::SwapPostProcess() {
        if (out_postprocess_idx == 0) {
            out_postprocess_idx = 1;
        } else {
            out_postprocess_idx = 0;
        }
    }

    blast::GfxTexture* View::GetInPostProcessRT() {
        if (out_postprocess_idx == 0) {
            return postprocess_rt1;
        } else {
            return postprocess_rt0;
        }
    }

    blast::GfxTexture* View::GetOutPostProcessRT() {
        if (out_postprocess_idx == 0) {
            return postprocess_rt0;
        } else {
            return postprocess_rt1;
        }
    }

    blast::GfxRenderPass* View::GetFXAARenderPass() {
        if (out_postprocess_idx == 0) {
            return fxaa_renderpass0;
        } else {
            return fxaa_renderpass1;
        }
    }

    blast::GfxRenderPass* View::GetDebugRenderPass() {
        if (out_postprocess_idx == 0) {
            return debug_renderpass0;
        } else {
            return debug_renderpass1;
        }
    }

	blast::GfxRenderPass* View::GetAtmosphereRaymarchingRenderPass() {
		if (out_postprocess_idx == 0) {
			return atmosphere_raymarching_renderpass0;
		} else {
			return atmosphere_raymarching_renderpass1;
		}
	}

    void View::AddDebugLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& c) {
        debug_lines[num_debug_lines * 14 + 0] = p0.x;
        debug_lines[num_debug_lines * 14 + 1] = p0.y;
        debug_lines[num_debug_lines * 14 + 2] = p0.z;
        debug_lines[num_debug_lines * 14 + 3] = c.x;
        debug_lines[num_debug_lines * 14 + 4] = c.y;
        debug_lines[num_debug_lines * 14 + 5] = c.z;
        debug_lines[num_debug_lines * 14 + 6] = c.w;
        debug_lines[num_debug_lines * 14 + 7] = p1.x;
        debug_lines[num_debug_lines * 14 + 8] = p1.y;
        debug_lines[num_debug_lines * 14 + 9] = p1.z;
        debug_lines[num_debug_lines * 14 + 10] = c.x;
        debug_lines[num_debug_lines * 14 + 11] = c.y;
        debug_lines[num_debug_lines * 14 + 12] = c.z;
        debug_lines[num_debug_lines * 14 + 13] = c.w;
        num_debug_lines++;
    }

    void View::DrawDebugBox(const glm::vec3* corners, const glm::vec4& c) {
        AddDebugLine(corners[0], corners[1], c);
        AddDebugLine(corners[1], corners[3], c);
        AddDebugLine(corners[3], corners[2], c);
        AddDebugLine(corners[2], corners[0], c);

        AddDebugLine(corners[4], corners[5], c);
        AddDebugLine(corners[5], corners[7], c);
        AddDebugLine(corners[7], corners[6], c);
        AddDebugLine(corners[6], corners[4], c);

        AddDebugLine(corners[0], corners[4], c);
        AddDebugLine(corners[1], corners[5], c);
        AddDebugLine(corners[2], corners[6], c);
        AddDebugLine(corners[3], corners[7], c);
    }

}