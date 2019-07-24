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

/**
  方便对RT进行操作
*/
enum class ClearMaskBits
{
	CLEAR_NONE = 0,
	CLEAR_COLOR0 = 1 << 0,
	CLEAR_COLOR1 = 1 << 1,
	CLEAR_COLOR2 = 1 << 2,
	CLEAR_COLOR3 = 1 << 3,
	CLEAR_COLOR4 = 1 << 4,
	CLEAR_COLOR5 = 1 << 5,
	CLEAR_COLOR6 = 1 << 6,
	CLEAR_COLOR7 = 1 << 7,
	CLEAR_STENCIL = 1 << 30,
	CLEAR_DEPTH = 1 << 31,
	CLEAR_ALL = 0xFF
};

inline ClearMaskBits operator|(ClearMaskBits a, ClearMaskBits b)
{
	return static_cast<ClearMaskBits>(static_cast<int>(a) | static_cast<int>(b));
}

inline ClearMaskBits operator&(ClearMaskBits a, ClearMaskBits b)
{
	return static_cast<ClearMaskBits>(static_cast<int>(a) & static_cast<int>(b));
}

enum class LoadMaskBits
{
	LOAD_NONE = 0,
	LOAD_COLOR0 = 1 << 0,
	LOAD_COLOR1 = 1 << 1,
	LOAD_COLOR2 = 1 << 2,
	LOAD_COLOR3 = 1 << 3,
	LOAD_COLOR4 = 1 << 4,
	LOAD_COLOR5 = 1 << 5,
	LOAD_COLOR6 = 1 << 6,
	LOAD_COLOR7 = 1 << 7,
	LOAD_STENCIL = 1 << 30,
	LOAD_DEPTH = 1 << 31,
	LOAD_ALL = 0xFF
};

inline LoadMaskBits operator|(LoadMaskBits a, LoadMaskBits b)
{
	return static_cast<LoadMaskBits>(static_cast<int>(a) | static_cast<int>(b));
}

inline LoadMaskBits operator&(LoadMaskBits a, LoadMaskBits b)
{
	return static_cast<LoadMaskBits>(static_cast<int>(a) & static_cast<int>(b));
}

enum class StoreMaskBits
{
	STORE_NONE = 0,
	STORE_COLOR0 = 1 << 0,
	STORE_COLOR1 = 1 << 1,
	STORE_COLOR2 = 1 << 2,
	STORE_COLOR3 = 1 << 3,
	STORE_COLOR4 = 1 << 4,
	STORE_COLOR5 = 1 << 5,
	STORE_COLOR6 = 1 << 6,
	STORE_COLOR7 = 1 << 7,
	STORE_STENCIL = 1 << 30,
	STORE_DEPTH = 1 << 31,
	STORE_ALL = 0xFF
};

inline StoreMaskBits operator|(StoreMaskBits a, StoreMaskBits b)
{
	return static_cast<StoreMaskBits>(static_cast<int>(a) | static_cast<int>(b));
}

inline StoreMaskBits operator&(StoreMaskBits a, StoreMaskBits b)
{
	return static_cast<StoreMaskBits>(static_cast<int>(a) & static_cast<int>(b));
}

#endif