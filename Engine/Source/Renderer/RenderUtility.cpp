#include "Renderer.h"
#include "GearEngine.h"
#include "RenderCache.h"
#include "View/View.h"
#include "Window/BaseWindow.h"
#include "Entity/Scene.h"
#include "UI/Canvas.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Resource/BuiltinResources.h"

#include <Blast/Gfx/GfxDevice.h>

namespace gear {
    struct PanoToCubeConstant {
        uint32_t mip;
        uint32_t max_size;
    };

    Texture* Renderer::EquirectangularMapToCubemap(Texture* equirectangular_map, uint32_t face_size) {
        gear::Texture::Builder builder;
        builder.SetWidth(face_size);
        builder.SetHeight(face_size);
        builder.SetLayer(6);
        builder.SetFormat(blast::FORMAT_R32G32B32A32_FLOAT); //equirectangular_map->GetTexture()->desc.format
        builder.SetCube(true);
        gear::Texture* cube_map = builder.Build();

        BuiltinResources* builtin_resources = gEngine.GetBuiltinResources();

        blast::GfxDevice* device = gEngine.GetDevice();

        blast::GfxTextureBarrier barrier;
        barrier.texture = cube_map->GetTexture();
        barrier.new_state = blast::RESOURCE_STATE_UNORDERED_ACCESS;
        device->SetBarrier(current_cmd, 0, nullptr, 1, &barrier);

        PanoToCubeConstant constant;
        constant.mip = 0;
        constant.max_size = face_size;
        device->PushConstants(current_cmd, &constant, sizeof(PanoToCubeConstant));

        device->BindResource(current_cmd, equirectangular_map->GetTexture(), 0);

        blast::GfxSamplerDesc default_sampler = {};
        device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

        device->BindUAV(current_cmd, cube_map->GetTexture(), 0);

        device->BindComputeShader(current_cmd, builtin_resources->GetPanoToCubeShader());

        device->Dispatch(current_cmd, std::max(1u, (uint32_t)(face_size) / 16), std::max(1u, (uint32_t)(face_size) / 16), 6);

        barrier.texture = cube_map->GetTexture();
        barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(current_cmd, 0, nullptr, 1, &barrier);

        return cube_map;
    }
}