#ifndef RHI_DEFINE_H
#define RHI_DEFINE_H
#include <vulkan/vulkan.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vk_sdk_platform.h>

#ifdef NDEBUG
#else
#define ENABLE_VALIDATION_LAYERS 1
#endif

//获取函数地址
namespace rhi
{
#define GET_INSTANCE_PROC_ADDR(instance, name) \
	vk##name = (PFN_vk##name)vkGetInstanceProcAddr(instance, "vk"##name);

#define GET_DEVICE_PROC_ADDR(device, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetDeviceProcAddr(device, "vk"#name));
}

#endif