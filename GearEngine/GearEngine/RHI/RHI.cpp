#include "RHI.h"
#include <assert.h>
#include <sstream>
#include "Utility/Log.h"
//自定义扩展函数
PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = nullptr;

//debug信息的回调函数
VkBool32 debugMsgCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
	size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData)
{
	std::stringstream  message;

	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		message << "ERROR";

	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		message << "WARNING";

	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		message << "PERFORMANCE";

	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		message << "INFO";

	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		message << "DEBUG";

	message << ": [" << pLayerPrefix << "] Code " << msgCode << ": " << pMsg << std::endl;

	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		LOGE("%s",message.str().c_str());
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT || flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		LOGW("%s", message.str().c_str());
	else
		LOGI("%s", message.str().c_str());

	return VK_FALSE;
}

RHI::RHI()
{
	createInstance();
	setupDebugCallback();
	pickPhysicalDevice();
	createLogicalDevice();
}

RHI::~RHI()
{
}

uint32_t RHI::findMemoryType(const uint32_t & typeFilter, const VkMemoryPropertyFlags & properties)
{
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(mGPU, &physicalDeviceMemoryProperties);

	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) 
	{
		if ((typeFilter & (1 << i)) &&
			(physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) 
		{
			return i;
		}
	}

	assert(false && "Failed to find a valid memory type for buffer!");
	return 0;
}

void RHI::createInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "GearEngine App";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "GearEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

#if ENABLE_VALIDATION_LAYERS
	const char* layers[] =
	{
		"VK_LAYER_LUNARG_standard_validation"
	};

	const char* extensions[] =
	{
		nullptr,
		nullptr,
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME
	};

	uint32_t numLayers = sizeof(layers) / sizeof(layers[0]);
#else
	const char** layers = nullptr;
	const char* extensions[] =
	{
		nullptr,
		nullptr,
	};

	uint32_t numLayers = 0;
#endif
	extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
	//目前只支持win平台
	extensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
	uint32_t numExtensions = sizeof(extensions) / sizeof(extensions[0]);

	VkInstanceCreateInfo instanceInfo;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = nullptr;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = numLayers;
	instanceInfo.ppEnabledLayerNames = layers;
	instanceInfo.enabledExtensionCount = numExtensions;
	instanceInfo.ppEnabledExtensionNames = extensions;

	if (vkCreateInstance(&instanceInfo, nullptr, &mInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
}

void RHI::pickPhysicalDevice()
{
	uint32_t mGpuCount = 0;
	if (vkEnumeratePhysicalDevices(mInstance, &mGpuCount, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to find mGpu!");
		return ;
	}

	if (mGpuCount == 0)
	{
		throw std::runtime_error("failed to find mGpu!");
		return ;
	}

	std::vector<VkPhysicalDevice> gpus(mGpuCount);
	if (vkEnumeratePhysicalDevices(mInstance, &mGpuCount, gpus.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to find mGpu!");
		return ;
	}

	for (auto &g : gpus)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(g, &props);
		LOGI("Found Vulkan GPU: %s\n", props.deviceName);
		LOGI("    API: %u.%u.%u\n",
			VK_VERSION_MAJOR(props.apiVersion),
			VK_VERSION_MINOR(props.apiVersion),
			VK_VERSION_PATCH(props.apiVersion));
		LOGI("    Driver: %u.%u.%u\n",
			VK_VERSION_MAJOR(props.driverVersion),
			VK_VERSION_MINOR(props.driverVersion),
			VK_VERSION_PATCH(props.driverVersion));
	}

	//默认选择第一块显卡
	mGPU = gpus.front();
}

void RHI::createLogicalDevice()
{
	
}

void RHI::setupDebugCallback()
{
#if ENABLE_VALIDATION_LAYERS

	GET_INSTANCE_PROC_ADDR(mInstance, CreateDebugReportCallbackEXT);
	GET_INSTANCE_PROC_ADDR(mInstance, DestroyDebugReportCallbackEXT);

	VkDebugReportFlagsEXT debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

	VkDebugReportCallbackCreateInfoEXT debugInfo;
	debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debugInfo.pNext = nullptr;
	debugInfo.flags = 0;
	debugInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugMsgCallback;
	debugInfo.flags = debugFlags;

	if (vkCreateDebugReportCallbackEXT(mInstance, &debugInfo, nullptr, &mDebugCallback) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug callback!");
	}
#endif
}

