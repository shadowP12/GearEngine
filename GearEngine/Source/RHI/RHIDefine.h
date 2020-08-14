#ifndef RHI_DEFINE_H
#define RHI_DEFINE_H
#pragma warning( disable : 26812)
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vk_sdk_platform.h>
#include <assert.h>

#define ENABLE_VALIDATION_LAYERS 1

#define CHECK_VKRESULT(exp)                                                    \
{                                                                              \
	VkResult vkres = (exp);                                                    \
	if (VK_SUCCESS != vkres)                                                   \
	{                                                                          \
		printf("%s: FAILED with VkResult: %u", #exp, (uint32_t)vkres);         \
		assert(false);                                                         \
	}                                                                          \
}

#define SAFE_DELETE(x) \
    { \
        delete x; \
        x = NULL; \
    }

#define SAFE_DELETE_ARRAY(x) \
    { \
        delete[] x; \
        x = NULL; \
    }

#ifdef __cplusplus
#ifndef MAKE_ENUM_FLAG
#define MAKE_ENUM_FLAG(TYPE, ENUM_TYPE)                                                                        \
	static inline ENUM_TYPE operator|(ENUM_TYPE a, ENUM_TYPE b) { return (ENUM_TYPE)((TYPE)(a) | (TYPE)(b)); } \
	static inline ENUM_TYPE operator&(ENUM_TYPE a, ENUM_TYPE b) { return (ENUM_TYPE)((TYPE)(a) & (TYPE)(b)); } \
	static inline ENUM_TYPE operator|=(ENUM_TYPE& a, ENUM_TYPE b) { return a = (a | b); }                      \
	static inline ENUM_TYPE operator&=(ENUM_TYPE& a, ENUM_TYPE b) { return a = (a & b); }
#endif
#else
#define MAKE_ENUM_FLAG(TYPE, ENUM_TYPE)
#endif

typedef enum ResourceState
{
    RESOURCE_STATE_UNDEFINED = 0,
    RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
    RESOURCE_STATE_INDEX_BUFFER = 0x2,
    RESOURCE_STATE_RENDER_TARGET = 0x4,
    RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
    RESOURCE_STATE_DEPTH_WRITE = 0x10,
    RESOURCE_STATE_DEPTH_READ = 0x20,
    RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
    RESOURCE_STATE_SHADER_RESOURCE = 0x40 | 0x80,
    RESOURCE_STATE_STREAM_OUT = 0x100,
    RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
    RESOURCE_STATE_COPY_DEST = 0x400,
    RESOURCE_STATE_COPY_SOURCE = 0x800,
    RESOURCE_STATE_GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
    RESOURCE_STATE_PRESENT = 0x1000,
    RESOURCE_STATE_COMMON = 0x2000,
    RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE = 0x4000,
} ResourceState;
MAKE_ENUM_FLAG(uint32_t, ResourceState)

typedef enum ResourceMemoryUsage
{
    RESOURCE_MEMORY_USAGE_UNKNOWN = 0,
    RESOURCE_MEMORY_USAGE_GPU_ONLY = 1,
    RESOURCE_MEMORY_USAGE_CPU_ONLY = 2,
    RESOURCE_MEMORY_USAGE_CPU_TO_GPU = 3,
    RESOURCE_MEMORY_USAGE_GPU_TO_CPU = 4,
    RESOURCE_MEMORY_USAGE_COUNT,
    RESOURCE_MEMORY_USAGE_MAX_ENUM = 0x7FFFFFFF
} ResourceMemoryUsage;

typedef enum DescriptorType
{
    DESCRIPTOR_TYPE_UNDEFINED = 0,
    DESCRIPTOR_TYPE_SAMPLER = 0x01,
    DESCRIPTOR_TYPE_TEXTURE = (DESCRIPTOR_TYPE_SAMPLER << 1),
    DESCRIPTOR_TYPE_RW_TEXTURE = (DESCRIPTOR_TYPE_TEXTURE << 1),
    DESCRIPTOR_TYPE_BUFFER = (DESCRIPTOR_TYPE_RW_TEXTURE << 1),
    DESCRIPTOR_TYPE_BUFFER_RAW = (DESCRIPTOR_TYPE_BUFFER | (DESCRIPTOR_TYPE_BUFFER << 1)),
    DESCRIPTOR_TYPE_RW_BUFFER = (DESCRIPTOR_TYPE_BUFFER << 2),
    DESCRIPTOR_TYPE_RW_BUFFER_RAW = (DESCRIPTOR_TYPE_RW_BUFFER | (DESCRIPTOR_TYPE_RW_BUFFER << 1)),
    DESCRIPTOR_TYPE_UNIFORM_BUFFER = (DESCRIPTOR_TYPE_RW_BUFFER << 2),
    DESCRIPTOR_TYPE_ROOT_CONSTANT = (DESCRIPTOR_TYPE_UNIFORM_BUFFER << 1),
    DESCRIPTOR_TYPE_VERTEX_BUFFER = (DESCRIPTOR_TYPE_ROOT_CONSTANT << 1),
    DESCRIPTOR_TYPE_INDEX_BUFFER = (DESCRIPTOR_TYPE_VERTEX_BUFFER << 1),
    DESCRIPTOR_TYPE_INDIRECT_BUFFER = (DESCRIPTOR_TYPE_INDEX_BUFFER << 1),
    DESCRIPTOR_TYPE_TEXTURE_CUBE = (DESCRIPTOR_TYPE_TEXTURE | (DESCRIPTOR_TYPE_INDIRECT_BUFFER << 1)),
    DESCRIPTOR_TYPE_RENDER_TARGET_MIP_SLICES = (DESCRIPTOR_TYPE_INDIRECT_BUFFER << 2),
    DESCRIPTOR_TYPE_RENDER_TARGET_ARRAY_SLICES = (DESCRIPTOR_TYPE_RENDER_TARGET_MIP_SLICES << 1),
    DESCRIPTOR_TYPE_RENDER_TARGET_DEPTH_SLICES = (DESCRIPTOR_TYPE_RENDER_TARGET_ARRAY_SLICES << 1),
    DESCRIPTOR_TYPE_RAY_TRACING = (DESCRIPTOR_TYPE_RENDER_TARGET_DEPTH_SLICES << 1),
	DESCRIPTOR_TYPE_INPUT_ATTACHMENT = (DESCRIPTOR_TYPE_RAY_TRACING << 1),
	DESCRIPTOR_TYPE_TEXEL_BUFFER = (DESCRIPTOR_TYPE_INPUT_ATTACHMENT << 1),
	DESCRIPTOR_TYPE_RW_TEXEL_BUFFER = (DESCRIPTOR_TYPE_TEXEL_BUFFER << 1),
} DescriptorType;
MAKE_ENUM_FLAG(uint32_t, DescriptorType)

VkBufferUsageFlags toVkBufferUsage(DescriptorType usage);

VkImageUsageFlags toVkImageUsage(DescriptorType usage);

VkAccessFlags toVkAccessFlags(ResourceState state);

VkImageLayout toVkImageLayout(ResourceState usage);

VkImageAspectFlags toVkAspectMask(VkFormat format);

inline uint32_t getBitSizeOfBlock(VkFormat fmt)
{
    switch(fmt) {
        case VK_FORMAT_UNDEFINED: return 0;
        case VK_FORMAT_R8_UNORM: return 8;
        case VK_FORMAT_R8_SNORM: return 8;
        case VK_FORMAT_R8_UINT: return 8;
        case VK_FORMAT_R8_SINT: return 8;
        case VK_FORMAT_R8_SRGB: return 8;
        case VK_FORMAT_R8G8_UNORM: return 16;
        case VK_FORMAT_R8G8_SNORM: return 16;
        case VK_FORMAT_R8G8_UINT: return 16;
        case VK_FORMAT_R8G8_SINT: return 16;
        case VK_FORMAT_R8G8_SRGB: return 16;
        case VK_FORMAT_R16_UNORM: return 16;
        case VK_FORMAT_R16_SNORM: return 16;
        case VK_FORMAT_R16_UINT: return 16;
        case VK_FORMAT_R16_SINT: return 16;
        case VK_FORMAT_R16_SFLOAT: return 16;
        case VK_FORMAT_R8G8B8_UNORM: return 24;
        case VK_FORMAT_R8G8B8_SNORM: return 24;
        case VK_FORMAT_R8G8B8_UINT: return 24;
        case VK_FORMAT_R8G8B8_SINT: return 24;
        case VK_FORMAT_R8G8B8_SRGB: return 24;
        case VK_FORMAT_B8G8R8_UNORM: return 24;
        case VK_FORMAT_B8G8R8_SNORM: return 24;
        case VK_FORMAT_B8G8R8_UINT: return 24;
        case VK_FORMAT_B8G8R8_SINT: return 24;
        case VK_FORMAT_B8G8R8_SRGB: return 24;
        case VK_FORMAT_R16G16B16_UNORM: return 48;
        case VK_FORMAT_R16G16B16_SNORM: return 48;
        case VK_FORMAT_R16G16B16_UINT: return 48;
        case VK_FORMAT_R16G16B16_SINT: return 48;
        case VK_FORMAT_R16G16B16_SFLOAT: return 48;
        case VK_FORMAT_R16G16B16A16_UNORM: return 64;
        case VK_FORMAT_R16G16B16A16_SNORM: return 64;
        case VK_FORMAT_R16G16B16A16_UINT: return 64;
        case VK_FORMAT_R16G16B16A16_SINT: return 64;
        case VK_FORMAT_R16G16B16A16_SFLOAT: return 64;
        case VK_FORMAT_R32G32_UINT: return 64;
        case VK_FORMAT_R32G32_SINT: return 64;
        case VK_FORMAT_R32G32_SFLOAT: return 64;
        case VK_FORMAT_R32G32B32_UINT: return 96;
        case VK_FORMAT_R32G32B32_SINT: return 96;
        case VK_FORMAT_R32G32B32_SFLOAT: return 96;
        case VK_FORMAT_R32G32B32A32_UINT: return 128;
        case VK_FORMAT_R32G32B32A32_SINT: return 128;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return 128;
        case VK_FORMAT_R64_UINT: return 64;
        case VK_FORMAT_R64_SINT: return 64;
        case VK_FORMAT_R64_SFLOAT: return 64;
        case VK_FORMAT_R64G64_UINT: return 128;
        case VK_FORMAT_R64G64_SINT: return 128;
        case VK_FORMAT_R64G64_SFLOAT: return 128;
        case VK_FORMAT_R64G64B64_UINT: return 192;
        case VK_FORMAT_R64G64B64_SINT: return 192;
        case VK_FORMAT_R64G64B64_SFLOAT: return 192;
        case VK_FORMAT_R64G64B64A64_UINT: return 256;
        case VK_FORMAT_R64G64B64A64_SINT: return 256;
        case VK_FORMAT_R64G64B64A64_SFLOAT: return 256;
        case VK_FORMAT_D16_UNORM: return 16;
        case VK_FORMAT_S8_UINT: return 8;
        case VK_FORMAT_D32_SFLOAT_S8_UINT: return 64;
        default: return 32;
    }
}

enum ProgramType
{
    PROGRAM_VERTEX,
    PROGRAM_FRAGMENT,
    PROGRAM_COMPUTE
};

enum QueueType
{
    QUEUE_TYPE_GRAPHICS,
    QUEUE_TYPE_COMPUTE,
    QUEUE_TYPE_TRANSFER
};

VkPipelineStageFlags toPipelineStageFlags(VkAccessFlags accessFlags, QueueType queueType);

#endif