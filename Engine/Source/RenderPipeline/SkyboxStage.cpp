#include "RenderPipeline.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Resource/RenderTarget.h"
#include "Resource/BuiltinResources.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
namespace gear {
    void RenderPipeline::ExecSkyboxStage() {
        Renderer* renderer = gEngine.GetRenderer();
        BuiltinResources* builtin_resources = gEngine.GetBuiltinResources();

        if (_has_skybox) {
            // 去掉位移信息
            glm::mat4 view = glm::mat4(glm::mat3(_display_camera_info.view));
            _view_ub->Update(&view, offsetof(ViewUniforms, view_matrix), sizeof(glm::mat4));

            renderer->BindFramebuffer(_display_fb);

            renderer->ResetUniformBufferSlot();
            renderer->BindFrameUniformBuffer(_view_ub->GetHandle(), _view_ub->GetSize(), 0);
            renderer->BindRenderableUniformBuffer(_debug_ub->GetHandle(), _debug_ub->GetSize(), 0);

            renderer->ResetSamplerSlot();

            blast::GfxSamplerDesc material_sampler_desc;

            SamplerInfo material_sampler_info = {};
            material_sampler_info.slot = 4;
            material_sampler_info.layer = 0;
            material_sampler_info.num_layers = 6;
            material_sampler_info.texture = _skybox_map->GetTexture();
            material_sampler_info.sampler_desc = material_sampler_desc;
            renderer->BindSampler(material_sampler_info);

            renderer->BindVertexShader(builtin_resources->GetSkyBoxMaterial()->GetVertShader(0));
            renderer->BindFragmentShader(builtin_resources->GetSkyBoxMaterial()->GetFragShader(0));

            renderer->SetDepthState(false, false);
            renderer->SetBlendingMode(BLENDING_MODE_OPAQUE);
            renderer->SetFrontFace(blast::FRONT_FACE_CCW);
            renderer->SetCullMode(blast::CULL_MODE_NONE);
            renderer->SetPrimitiveTopo(blast::PRIMITIVE_TOPO_TRI_LIST);

            VertexBuffer* cube_buffer = builtin_resources->GetCubeBuffer();
            renderer->BindVertexBuffer(cube_buffer->GetHandle(), cube_buffer->GetVertexLayout(), cube_buffer->GetSize(), 0);

            renderer->Draw(36, 0);

            renderer->UnbindFramebuffer();

            _view_ub->Update(&_display_camera_info.view, offsetof(ViewUniforms, view_matrix), sizeof(glm::mat4));
        }
    }
}
