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

// CSM的Cascade个数
#define SHADOW_CASCADE_COUNT 1

// 阴影贴图个数
#define SHADOW_MAP_COUNT 4

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