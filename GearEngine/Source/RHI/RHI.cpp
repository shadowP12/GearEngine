#include "RHI.h"
#include <assert.h>
#include <sstream>
#include "Utility/Log.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

//VkBool32 debugMsgCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
//	size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData)
//{
//	std::stringstream  message;
//
//	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
//		message << "ERROR";
//
//	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
//		message << "WARNING";
//
//	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
//		message << "PERFORMANCE";
//
//	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
//		message << "INFO";
//
//	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
//		message << "DEBUG";
//
//	message << ": [" << pLayerPrefix << "] Code " << msgCode << ": " << pMsg << std::endl;
//
//	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
//		LOGE("%s",message.str().c_str());
//	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT || flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
//		LOGW("%s", message.str().c_str());
//	else
//		LOGI("%s", message.str().c_str());
//
//	return VK_FALSE;
//}

RHI::RHI()
{
	createInstance();
	setupDebugCallback();
	pickPhysicalDevice();
	createLogicalDevice();
}

RHI::~RHI()
{
#if ENABLE_VALIDATION_LAYERS
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(mInstance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(mInstance, mDebugMessenger, nullptr);
    }
#endif
	if (mActiceDevice)
		delete mActiceDevice;

	vkDestroyInstance(mInstance, nullptr);
}

void RHI::createInstance()
{
    std::vector<const char*> instanceLayers;
    std::vector<const char*> instanceExtensions;
#if ENABLE_VALIDATION_LAYERS
    instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "GearEngine App";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
	appInfo.pEngineName = "GearEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 2, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo instanceInfo;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = nullptr;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = instanceLayers.size();
	instanceInfo.ppEnabledLayerNames = instanceLayers.data();
	instanceInfo.enabledExtensionCount = instanceExtensions.size();
	instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();

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
	mActiceDevice = new RHIDevice(mGPU);
}

void RHI::setupDebugCallback()
{
#if ENABLE_VALIDATION_LAYERS
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(mInstance, "vkCreateDebugUtilsMessengerEXT");
    if (func(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
#endif
}

