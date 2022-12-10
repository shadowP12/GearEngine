#include "Renderer.h"
#include "GearEngine.h"
#include "RenderCache.h"
#include "View/View.h"
#include "Window/BaseWindow.h"
#include "Entity/Scene.h"
#include "UI/Canvas.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Resource/BuiltinResources.h"

#include <GfxDevice.h>

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

    std::shared_ptr<blast::GfxTexture> Renderer::EquirectangularMapToCubemap(std::shared_ptr<blast::GfxTexture> equirectangular_map, uint32_t face_size) {
        BuiltinResources* builtin_resources = gEngine.GetBuiltinResources();

        blast::GfxTextureDesc texture_desc {};
        texture_desc.width = face_size;
        texture_desc.height = face_size;
        texture_desc.num_layers = 6;
        texture_desc.format = blast::FORMAT_R32G32B32A32_FLOAT;
        texture_desc.res_usage = blast::RESOURCE_USAGE_SHADER_RESOURCE | blast::RESOURCE_USAGE_RENDER_TARGET | blast::RESOURCE_USAGE_UNORDERED_ACCESS | blast::RESOURCE_USAGE_CUBE_TEXTURE;
        texture_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
        std::shared_ptr<blast::GfxTexture> cube_map = std::shared_ptr<blast::GfxTexture>(device->CreateTexture(texture_desc));

        blast::GfxResourceBarrier barrier;
        barrier.resource = cube_map.get();
        barrier.new_state = blast::RESOURCE_STATE_UNORDERED_ACCESS;
        device->SetBarrier(current_cmd, 1, &barrier);

        PanoToCubeConstant constant;
        constant.mip = 0;
        constant.max_size = face_size;
        device->PushConstants(current_cmd, &constant, sizeof(PanoToCubeConstant));

        device->BindResource(current_cmd, equirectangular_map.get(), 0);

        blast::GfxSamplerDesc default_sampler = {};
        device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

        device->BindUAV(current_cmd, cube_map.get(), 0);

        device->BindComputeShader(current_cmd, builtin_resources->GetPanoToCubeShader().get());

        device->Dispatch(current_cmd, std::max(1u, (uint32_t)(face_size) / 16), std::max(1u, (uint32_t)(face_size) / 16), 6);

        barrier.resource = cube_map.get();
        barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(current_cmd, 1, &barrier);

        return cube_map;
    }

    std::shared_ptr<blast::GfxTexture> Renderer::ComputeIrradianceMap(std::shared_ptr<blast::GfxTexture> cube_map) {
        uint32_t irradiance_map_size = 64;
        BuiltinResources* builtin_resources = gEngine.GetBuiltinResources();

        blast::GfxTextureDesc texture_desc {};
        texture_desc.width = irradiance_map_size;
        texture_desc.height = irradiance_map_size;
        texture_desc.num_layers = 6;
        texture_desc.format = blast::FORMAT_R32G32B32A32_FLOAT;
        texture_desc.res_usage = blast::RESOURCE_USAGE_SHADER_RESOURCE | blast::RESOURCE_USAGE_RENDER_TARGET | blast::RESOURCE_USAGE_UNORDERED_ACCESS | blast::RESOURCE_USAGE_CUBE_TEXTURE;
        texture_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
        std::shared_ptr<blast::GfxTexture> irradiance_map = std::shared_ptr<blast::GfxTexture>(device->CreateTexture(texture_desc));

        blast::GfxResourceBarrier barrier;
        barrier.resource = irradiance_map.get();
        barrier.new_state = blast::RESOURCE_STATE_UNORDERED_ACCESS;
        device->SetBarrier(current_cmd, 1, &barrier);

        ComputeIrradianceConstant constant;
        constant.mip = 0;
        constant.max_size = irradiance_map_size;
        device->PushConstants(current_cmd, &constant, sizeof(ComputeIrradianceConstant));

        device->BindResource(current_cmd, cube_map.get(), 0);

        blast::GfxSamplerDesc default_sampler = {};
        default_sampler.address_u = blast::ADDRESS_MODE_CLAMP_TO_EDGE;
        default_sampler.address_v = blast::ADDRESS_MODE_CLAMP_TO_EDGE;
        default_sampler.address_w = blast::ADDRESS_MODE_CLAMP_TO_EDGE;
        device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

        device->BindUAV(current_cmd, irradiance_map.get(), 0);

        device->BindComputeShader(current_cmd, builtin_resources->GetComputeIrradianceMapShader().get());

        device->Dispatch(current_cmd, std::max(1u, (uint32_t)(irradiance_map_size) / 16), std::max(1u, (uint32_t)(irradiance_map_size) / 16), 6);

        barrier.resource = irradiance_map.get();
        barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(current_cmd, 1, &barrier);

        return irradiance_map;
    }

    std::shared_ptr<blast::GfxTexture> Renderer::ComputePrefilteredMap(std::shared_ptr<blast::GfxTexture> cube_map) {
        uint32_t mip_level_count = 5;
        uint32_t prefiltered_map_size = 128;
        BuiltinResources* builtin_resources = gEngine.GetBuiltinResources();

        blast::GfxTextureDesc texture_desc {};
        texture_desc.width = prefiltered_map_size;
        texture_desc.height = prefiltered_map_size;
        texture_desc.num_layers = 6;
        texture_desc.num_levels = mip_level_count;
        texture_desc.format = blast::FORMAT_R32G32B32A32_FLOAT;
        texture_desc.res_usage = blast::RESOURCE_USAGE_SHADER_RESOURCE | blast::RESOURCE_USAGE_RENDER_TARGET | blast::RESOURCE_USAGE_UNORDERED_ACCESS | blast::RESOURCE_USAGE_CUBE_TEXTURE;
        texture_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
        std::shared_ptr<blast::GfxTexture> prefiltered_map = std::shared_ptr<blast::GfxTexture>(device->CreateTexture(texture_desc));

        blast::GfxDevice* device = gEngine.GetRenderer()->GetDevice();

        blast::GfxResourceBarrier barrier;
        barrier.resource = prefiltered_map.get();
        barrier.new_state = blast::RESOURCE_STATE_UNORDERED_ACCESS;
        device->SetBarrier(current_cmd, 1, &barrier);

        for (uint32_t i = 0; i < mip_level_count; ++i) {
            uint32_t mip_size = prefiltered_map_size >> i;
            int32_t uav_sub_resource = device->CreateSubresource(prefiltered_map.get(), blast::UAV, 0, 6, i, 1);

            ComputeSpecularConstant constant;
            constant.roughness = (float)i / (float)(mip_level_count - 1);
            constant.mip_level = i;
            constant.max_size = prefiltered_map_size;
            device->PushConstants(current_cmd, &constant, sizeof(ComputeSpecularConstant));

            device->BindResource(current_cmd, cube_map.get(), 0);

            blast::GfxSamplerDesc default_sampler = {};
            device->BindSampler(current_cmd, sampler_cache->GetSampler(default_sampler), 0);

            device->BindUAV(current_cmd, prefiltered_map.get(), 0, uav_sub_resource);

            device->BindComputeShader(current_cmd, builtin_resources->GetComputeSpecularMapShader().get());

            device->Dispatch(current_cmd, std::max(1u, (uint32_t)(mip_size) / 16), std::max(1u, (uint32_t)(mip_size) / 16), 6);
        }

        barrier.resource = prefiltered_map.get();
        barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(current_cmd, 1, &barrier);

        return prefiltered_map;
    }

    std::shared_ptr<blast::GfxTexture> Renderer::ComputeBRDFLut() {
        BuiltinResources* builtin_resources = gEngine.GetBuiltinResources();

        blast::GfxTextureDesc texture_desc {};
        texture_desc.width = 512;
        texture_desc.height = 512;
        texture_desc.format = blast::FORMAT_R32G32_FLOAT;
        texture_desc.res_usage = blast::RESOURCE_USAGE_SHADER_RESOURCE | blast::RESOURCE_USAGE_RENDER_TARGET | blast::RESOURCE_USAGE_UNORDERED_ACCESS;
        texture_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
        std::shared_ptr<blast::GfxTexture> lut = std::shared_ptr<blast::GfxTexture>(device->CreateTexture(texture_desc));

        blast::GfxResourceBarrier barrier;
        barrier.resource = lut.get();
        barrier.new_state = blast::RESOURCE_STATE_UNORDERED_ACCESS;
        device->SetBarrier(current_cmd, 1, &barrier);

        BRDFIntegrationConstant constant;
        constant.lut_size = 512;
        device->PushConstants(current_cmd, &constant, sizeof(BRDFIntegrationConstant));

        device->BindUAV(current_cmd, lut.get(), 0);

        device->BindComputeShader(current_cmd, builtin_resources->GetBRDFIntegrationShader().get());

        device->Dispatch(current_cmd, std::max(1u, (uint32_t)(512) / 16), std::max(1u, (uint32_t)(512) / 16), 6);

        barrier.resource = lut.get();
        barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(current_cmd, 1, &barrier);

        return lut;
    }
}