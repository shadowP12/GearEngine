#ifndef VULKAN_PREREQUISITES_H
#define VULKAN_PREREQUISITES_H
#include <vulkan/vulkan.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vk_sdk_platform.h>
enum GpuBufferUsage
{
	GBU_STATIC = 1 << 0,
	GBU_DYNAMIC = 1 << 1,
	GBU_LOADSTORE = GBU_STATIC | 1 << 2
};

#endif