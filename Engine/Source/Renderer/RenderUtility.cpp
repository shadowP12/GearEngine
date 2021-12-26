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

    struct ComputeIrradianceConstant {
        uint32_t mip;
        uint32_t max_size;
    };

    struct ComputeSpecularConstant {
        uint32_t mip_level;
        uint32_t max_size;
        float roughness;
    };

    struct BRDFIntegrationConstant {
        uint32_t lut_size;
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

    Texture* Renderer::ComputeIrradianceMap(Texture* cube_map) {
        uint32_t irradiance_map_size = 64;

        gear::Texture::Builder builder;
        builder.SetWidth(irradiance_map_size);
        builder.SetHeight(irradiance_map_size);
        builder.SetLayer(6);
        builder.SetFormat(blast::FORMAT_R32G32B32A32_FLOAT);
        builder.SetCube(true);
        gear::Texture* irradiance_map = builder.Build();

        BuiltinResources* builtin_resources = gEngine.GetBuiltinResources();

        blast::GfxDevice* device = gEngine.GetDevice();

        blast::GfxTextureBarrier barrier;
        barrier.texture = irradiance_map->GetTexture();
        barrier.new_state = blast::RESOURCE_STATE_UNORDERED_ACCESS;
        device->SetBarrier(current_cmd, 0, nullptr, 1, &barrier);

        ComputeIrradianceConstant constant;
        constant.mip = 0;
        constant.max_size = irradiance_map_size;
        device->PushConstants(current_cmd, &constant, sizeof(ComputeIrradianceConstant));

        device->BindResource(current_cmd, cube_map->GetTexture(), 0);

        blast::GfxSamplerDesc default_sampler = {};
        default_sampler.address_u = blast::ADDRESS_MODE_CLAMP_TO_EDGE;
        default_sampler.address_v = blast::ADDRESS_MODE_CLAMP_TO_EDGE;
        default_sampler.address_w = blast::ADDRESS_MODE_CLAMP_TO_EDGE;
        device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

        device->BindUAV(current_cmd, irradiance_map->GetTexture(), 0);

        device->BindComputeShader(current_cmd, builtin_resources->GetComputeIrradianceMapShader());

        device->Dispatch(current_cmd, std::max(1u, (uint32_t)(irradiance_map_size) / 16), std::max(1u, (uint32_t)(irradiance_map_size) / 16), 6);

        barrier.texture = irradiance_map->GetTexture();
        barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(current_cmd, 0, nullptr, 1, &barrier);

        return irradiance_map;
    }

    Texture* Renderer::ComputePrefilteredMap(Texture* cube_map) {
        uint32_t mip_level_count = 5;
        uint32_t prefiltered_map_size = 128;

        gear::Texture::Builder builder;
        builder.SetWidth(prefiltered_map_size);
        builder.SetHeight(prefiltered_map_size);
        builder.SetLayer(6);
        builder.SetLevel(mip_level_count);
        builder.SetFormat(blast::FORMAT_R32G32B32A32_FLOAT);
        builder.SetCube(true);
        gear::Texture* prefiltered_map = builder.Build();

        BuiltinResources* builtin_resources = gEngine.GetBuiltinResources();

        blast::GfxDevice* device = gEngine.GetDevice();

        blast::GfxTextureBarrier barrier;
        barrier.texture = prefiltered_map->GetTexture();
        barrier.new_state = blast::RESOURCE_STATE_UNORDERED_ACCESS;
        device->SetBarrier(current_cmd, 0, nullptr, 1, &barrier);

        for (uint32_t i = 0; i < mip_level_count; ++i) {
            uint32_t mip_size = prefiltered_map_size >> i;
            int32_t uav_sub_resource = gEngine.GetDevice()->CreateSubresource(prefiltered_map->GetTexture(), blast::UAV, 0, 6, i, 1);

            ComputeSpecularConstant constant;
            constant.roughness = (float)i / (float)(mip_level_count - 1);
            constant.mip_level = i;
            constant.max_size = prefiltered_map_size;
            device->PushConstants(current_cmd, &constant, sizeof(ComputeSpecularConstant));

            device->BindResource(current_cmd, cube_map->GetTexture(), 0);

            blast::GfxSamplerDesc default_sampler = {};
            device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

            device->BindUAV(current_cmd, prefiltered_map->GetTexture(), 0, uav_sub_resource);

            device->BindComputeShader(current_cmd, builtin_resources->GetComputeSpecularMapShader());

            device->Dispatch(current_cmd, std::max(1u, (uint32_t)(mip_size) / 16), std::max(1u, (uint32_t)(mip_size) / 16), 6);
        }

        barrier.texture = prefiltered_map->GetTexture();
        barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(current_cmd, 0, nullptr, 1, &barrier);

        return prefiltered_map;
    }

    Texture* Renderer::ComputeBRDFLut() {
        gear::Texture::Builder builder;
        builder.SetWidth(512);
        builder.SetHeight(512);
        builder.SetFormat(blast::FORMAT_R32G32_FLOAT);
        gear::Texture* lut = builder.Build();

        BuiltinResources* builtin_resources = gEngine.GetBuiltinResources();

        blast::GfxDevice* device = gEngine.GetDevice();

        blast::GfxTextureBarrier barrier;
        barrier.texture = lut->GetTexture();
        barrier.new_state = blast::RESOURCE_STATE_UNORDERED_ACCESS;
        device->SetBarrier(current_cmd, 0, nullptr, 1, &barrier);

        BRDFIntegrationConstant constant;
        constant.lut_size = 512;
        device->PushConstants(current_cmd, &constant, sizeof(BRDFIntegrationConstant));

        device->BindUAV(current_cmd, lut->GetTexture(), 0);

        device->BindComputeShader(current_cmd, builtin_resources->GetBRDFIntegrationShader());

        device->Dispatch(current_cmd, std::max(1u, (uint32_t)(512) / 16), std::max(1u, (uint32_t)(512) / 16), 6);

        barrier.texture = lut->GetTexture();
        barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(current_cmd, 0, nullptr, 1, &barrier);

        return lut;
    }
}