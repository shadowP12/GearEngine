#include "RenderUtility.h"
#include "Renderer/Renderer.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Resource/BuiltinResources.h"
#include "GearEngine.h"
#include "Math/Math.h"

namespace gear {
    RenderUtility::RenderUtility() {
    }

    RenderUtility::~RenderUtility() {
    }

    Texture* RenderUtility::EquirectangularMapToCubemap(Texture* equirectangular_map, uint32_t face_size) {
        gear::Texture::Builder builder;
        builder.SetWidth(face_size);
        builder.SetHeight(face_size);
        builder.SetLayer(6);
        builder.SetFormat(equirectangular_map->GetTexture()->GetFormat());
        builder.SetCube(true);
        gear::Texture* cube_map = builder.Build();

        // 因为Vulkan无法直接使用CubeMap作为Attachment,所以这里需要借助一个离屏FB
        builder = {};
        builder.SetWidth(face_size);
        builder.SetHeight(face_size);
        builder.SetFormat(equirectangular_map->GetTexture()->GetFormat());
        gear::Texture* offscreen = builder.Build();

        Renderer* renderer = gEngine.GetRenderer();
        blast::GfxTexture* internal_equirectangular_map = equirectangular_map->GetTexture();
        blast::GfxTexture* internal_cube_map = cube_map->GetTexture();
        blast::GfxTexture* internal_offscreen = offscreen->GetTexture();
        renderer->EnqueueUploadTask([renderer, internal_equirectangular_map, internal_cube_map, internal_offscreen]() {

            glm::mat4 capture_projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
            // 翻转y轴
            capture_projection[1][1] *= -1;

            glm::mat4 capture_views[] =
                    {
                            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
                            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
                            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
                    };

            UniformBuffer* renderable_ub = new UniformBuffer(sizeof(RenderableUniforms));
            glm::mat4 identity_matrix = glm::mat4(1.0f);
            renderable_ub->Update(&identity_matrix, offsetof(RenderableUniforms, model_matrix), sizeof(glm::mat4));
            renderable_ub->Update(&identity_matrix, offsetof(RenderableUniforms, normal_matrix), sizeof(glm::mat4));

            UniformBuffer* view_ub = new UniformBuffer(sizeof(ViewUniforms));

            BuiltinResources* builtin_resources = gEngine.GetBuiltinResources();

            {
                // 设置纹理为读写状态
                blast::GfxTextureBarrier barrier;
                barrier.texture = internal_cube_map;
                barrier.new_state = blast::RESOURCE_STATE_COPY_DEST;
                renderer->SetBarrier(barrier);
            }

            for (uint32_t i = 0; i < 6; ++i) {
                view_ub->Update(&capture_projection, offsetof(ViewUniforms, proj_matrix), sizeof(glm::mat4));
                view_ub->Update(&capture_views[i], offsetof(ViewUniforms, view_matrix), sizeof(glm::mat4));

                FramebufferInfo capture_fb;
                capture_fb.clear_value.flags = blast::CLEAR_COLOR;
                capture_fb.clear_value.depth = 1.0f;
                capture_fb.width = internal_offscreen->GetWidth();
                capture_fb.height = internal_offscreen->GetHeight();
                capture_fb.viewport[0] = 0;
                capture_fb.viewport[1] = 0;
                capture_fb.viewport[2] = internal_offscreen->GetWidth();
                capture_fb.viewport[3] = internal_offscreen->GetHeight();
                capture_fb.colors[0].texture = internal_offscreen;
                capture_fb.colors[0].layer = 0;
                capture_fb.colors[0].num_layers = 1;
                capture_fb.colors[0].level = 0;
                capture_fb.colors[0].num_levels = 1;

                renderer->BindFramebuffer(capture_fb);

                renderer->ResetUniformBufferSlot();
                renderer->BindFrameUniformBuffer(view_ub->GetHandle(), view_ub->GetSize(), 0);
                renderer->BindRenderableUniformBuffer(renderable_ub->GetHandle(), renderable_ub->GetSize(), 0);

                renderer->ResetSamplerSlot();

                blast::GfxSamplerDesc material_sampler_desc;

                SamplerInfo material_sampler_info = {};
                material_sampler_info.slot = 4;
                material_sampler_info.layer = 0;
                material_sampler_info.num_layers = 1;
                material_sampler_info.texture = internal_equirectangular_map;
                material_sampler_info.sampler_desc = material_sampler_desc;
                renderer->BindSampler(material_sampler_info);

                renderer->BindVertexShader(builtin_resources->GetEquirectangularToCubeMaterial()->GetVertShader(0));
                renderer->BindFragmentShader(builtin_resources->GetEquirectangularToCubeMaterial()->GetFragShader(0));

                renderer->SetDepthState(true);
                renderer->SetBlendingMode(BLENDING_MODE_OPAQUE);
                renderer->SetFrontFace(blast::FRONT_FACE_CCW);
                renderer->SetCullMode(blast::CULL_MODE_NONE);
                renderer->SetPrimitiveTopo(blast::PRIMITIVE_TOPO_TRI_LIST);

                VertexBuffer* cube_buffer = builtin_resources->GetCubeBuffer();
                renderer->BindVertexBuffer(cube_buffer->GetHandle(), cube_buffer->GetVertexLayout(), cube_buffer->GetSize(), 0);

                renderer->Draw(36, 0);

                renderer->UnbindFramebuffer();

                // copy
                {
                    // 设置纹理为读写状态
                    blast::GfxTextureBarrier barrier;
                    barrier.texture = internal_offscreen;
                    barrier.new_state = blast::RESOURCE_STATE_COPY_SOURCE;
                    renderer->SetBarrier(barrier);
                }

                blast::GfxImageCopyToImageRange range;
                range.src_texture = internal_offscreen;
                range.dst_texture = internal_cube_map;
                range.width = internal_offscreen->GetWidth();
                range.height = internal_offscreen->GetHeight();
                range.depth = 1;
                range.src_layer = 0;
                range.src_level = 0;
                range.dst_layer = i;
                range.dst_level = 0;
                renderer->ImageCopyToImage(range);

                {
                    // 设置纹理为Shader可读状态
                    blast::GfxTextureBarrier barrier;
                    barrier.texture = internal_offscreen;
                    barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
                    renderer->SetBarrier(barrier);
                }
            }

            {
                // 设置纹理为Shader可读状态
                blast::GfxTextureBarrier barrier;
                barrier.texture = internal_cube_map;
                barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
                renderer->SetBarrier(barrier);
            }

            SAFE_DELETE(view_ub);
            SAFE_DELETE(renderable_ub);
        });

        SAFE_DELETE(offscreen);

        return cube_map;
    }
}