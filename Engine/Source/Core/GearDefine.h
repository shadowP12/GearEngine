#pragma once

#ifdef GEAR_EXPORTS
#define GEAR_EXPORT __declspec(dllexport)
#else
#define GEAR_EXPORT __declspec(dllimport)
#endif

// MRT的个数
#define TARGET_COUNT 4

// UniformBuffer个数
#define UBUFFER_BINDING_COUNT 3

// 纹理采样器个数
#define SAMPLER_BINDING_COUNT 16

// shader最大纹理数量
#define MAX_TEXTURE_COUNT 32

// CSM的Cascade个数
#define SHADOW_CASCADE_COUNT 3

// 阴影贴图个数
#define SHADOW_MAP_COUNT 4

// 最大的Primitive个数
#define MAX_RENDER_PRIMITIVE_COUNT 6

// Debug Line数量
#define MAX_DEBUG_LINES 1024

// Atmosphere
#define TRANSMITTANCE_TEXTURE_WIDTH 64
#define TRANSMITTANCE_TEXTURE_HEIGHT 16

#define SCATTERING_TEXTURE_R_SIZE 16
#define SCATTERING_TEXTURE_MU_SIZE 16
#define SCATTERING_TEXTURE_MU_S_SIZE 16
#define SCATTERING_TEXTURE_NU_SIZE 4

#define IRRADIANCE_TEXTURE_WIDTH 32;
#define IRRADIANCE_TEXTURE_HEIGHT 8;

#define SAFE_DELETE(x) \
    { \
        delete x; \
        x = nullptr; \
    }

#define SAFE_DELETE_ARRAY(x) \
    { \
        delete[] x; \
        x = nullptr; \
    }