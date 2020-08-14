#include "RHIDevice.h"
#include "RHIQueue.h"
#include "RHISynchronization.h"
#include "RHICommandBuffer.h"
#include <vector>
#include <stdexcept>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
    printf("validation layer: %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

RHIDevice::RHIDevice()
{
    // create instance
    createInstance();

    // setting debug callback
    setupDebugCallback();

    // pick gpu
    pickGPU();

	vkGetPhysicalDeviceProperties(mGPU, &mDeviceProperties);
	vkGetPhysicalDeviceFeatures(mGPU, &mDeviceFeatures);
	//预储存gpu内存属性
	vkGetPhysicalDeviceMemoryProperties(mGPU, &mMemoryProperties);

	//Note:这里有可能会出现三个queue的Family相同的情况
	uint32_t numQueueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties(mGPU, &numQueueFamilies, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyProperties(numQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(mGPU, &numQueueFamilies, queueFamilyProperties.data());

	uint32_t graphicsFamily = -1;
	uint32_t computeFamily = -1;
	uint32_t transferFamily = -1;

	for (uint32_t i = 0; i < (uint32_t)queueFamilyProperties.size(); i++)
	{
		if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
		{
			computeFamily = i;
			break;
		}
	}

	for (uint32_t i = 0; i < (uint32_t)queueFamilyProperties.size(); i++)
	{
		if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
			((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
			((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
		{
			transferFamily = i;
			break;
		}
	}

	for (uint32_t i = 0; i < (uint32_t)queueFamilyProperties.size(); i++)
	{
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsFamily = i;
			break;
		}
	}

	const float graphicsQueuePrio = 0.0f;
	const float computeQueuePrio = 0.1f;
	const float transferQueuePrio = 0.2f;

	std::vector<VkDeviceQueueCreateInfo> queueInfo{};
	queueInfo.resize(3);
	
	queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo[0].queueFamilyIndex = graphicsFamily;
	queueInfo[0].queueCount = 1;// queueFamilyProperties[graphicsFamily].queueCount;
	queueInfo[0].pQueuePriorities = &graphicsQueuePrio;

	queueInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo[1].queueFamilyIndex = computeFamily;
	queueInfo[1].queueCount = 1;// queueFamilyProperties[computeFamily].queueCount;
	queueInfo[1].pQueuePriorities = &computeQueuePrio;

	queueInfo[2].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo[2].queueFamilyIndex = transferFamily;
	queueInfo[2].queueCount = 1;// queueFamilyProperties[transferFamily].queueCount;
	queueInfo[2].pQueuePriorities = &transferQueuePrio;

	const char* extensions[5];
	uint32_t numExtensions = 0;

	extensions[numExtensions++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	extensions[numExtensions++] = VK_KHR_MAINTENANCE1_EXTENSION_NAME;
	extensions[numExtensions++] = VK_KHR_MAINTENANCE2_EXTENSION_NAME;

	bool dedicatedAllocExt = false;
	bool getMemReqExt = false;

	uint32_t numAvailableExtensions = 0;
	vkEnumerateDeviceExtensionProperties(mGPU, nullptr, &numAvailableExtensions, nullptr);
	if (numAvailableExtensions > 0)
	{
		std::vector<VkExtensionProperties> availableExtensions(numAvailableExtensions);
		if (vkEnumerateDeviceExtensionProperties(mGPU, nullptr, &numAvailableExtensions, availableExtensions.data()) == VK_SUCCESS)
		{
			for (auto entry : extensions)
			{
				if (entry == VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)
				{
					extensions[numExtensions++] = VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME;
					dedicatedAllocExt = true;
				}
				else if (entry == VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME)
				{
					extensions[numExtensions++] = VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME;
					getMemReqExt = true;
				}
			}
		}
	}

	VkDeviceCreateInfo deviceInfo ;
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = nullptr;
	deviceInfo.flags = 0;
	deviceInfo.queueCreateInfoCount = queueInfo.size();
	deviceInfo.pQueueCreateInfos = queueInfo.data();
	deviceInfo.pEnabledFeatures = &mDeviceFeatures;
	deviceInfo.enabledExtensionCount = numExtensions;
	deviceInfo.ppEnabledExtensionNames = extensions;
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = nullptr;

    CHECK_VKRESULT(vkCreateDevice(mGPU, &deviceInfo, nullptr, &mDevice));

	VkQueue graphicsQueue;
	VkQueue computeQueue;
	VkQueue transferQueue;

	vkGetDeviceQueue(mDevice, graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(mDevice, computeFamily, 0, &computeQueue);
	vkGetDeviceQueue(mDevice, transferFamily, 0, &transferQueue);

	mGraphicsQueue = new RHIQueue(this, graphicsQueue, graphicsFamily, QUEUE_TYPE_GRAPHICS);
	mComputeQueue = new RHIQueue(this, computeQueue, computeFamily, QUEUE_TYPE_COMPUTE);
	mTransferQueue = new RHIQueue(this, transferQueue, transferFamily, QUEUE_TYPE_TRANSFER);

	mGraphicsCommandPool = new RHICommandBufferPool(this, mGraphicsQueue, true);

	// 创建全局的描述符池
	uint32_t setCount                  = 65535;
    uint32_t sampledImageCount         = 32 * 65536;
    uint32_t storageImageCount         = 1  * 65536;
    uint32_t uniformBufferCount        = 1  * 65536;
    uint32_t uniformBufferDynamicCount = 4  * 65536;
    uint32_t storageBufferCount        = 1  * 65536;
    uint32_t uniformTexelBufferCount   = 8192;
    uint32_t storageTexelBufferCount   = 8192;
    uint32_t samplerCount              = 2  * 65536;

    VkDescriptorPoolSize poolSizes[8];

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSizes[0].descriptorCount = sampledImageCount;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[1].descriptorCount = storageImageCount;

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[2].descriptorCount = uniformBufferCount;

    poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[3].descriptorCount = uniformBufferDynamicCount;

    poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[4].descriptorCount = storageBufferCount;

    poolSizes[5].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSizes[5].descriptorCount = samplerCount;

    poolSizes[6].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    poolSizes[6].descriptorCount = uniformTexelBufferCount;

    poolSizes[7].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    poolSizes[7].descriptorCount = storageTexelBufferCount;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.pNext = nullptr;
    descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // Allocated descriptor sets will release their allocations back to the pool
    descriptorPoolCreateInfo.maxSets = setCount;
    descriptorPoolCreateInfo.poolSizeCount = 8;
    descriptorPoolCreateInfo.pPoolSizes = poolSizes;

    vkCreateDescriptorPool(mDevice, &descriptorPoolCreateInfo, nullptr, &mDescriptorPool);
}

RHIDevice::~RHIDevice()
{
	SAFE_DELETE(mGraphicsCommandPool);
	SAFE_DELETE(mGraphicsQueue);
	SAFE_DELETE(mComputeQueue);
	SAFE_DELETE(mTransferQueue);
    vkDeviceWaitIdle(mDevice);
    vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
	vkDestroyDevice(mDevice, nullptr);

#if ENABLE_VALIDATION_LAYERS
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(mInstance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(mInstance, mDebugMessenger, nullptr);
    }
#endif
    vkDestroyInstance(mInstance, nullptr);
}

uint32_t RHIDevice::findMemoryType(const uint32_t &typeFilter, const VkMemoryPropertyFlags &properties)
{
	for (uint32_t i = 0; i < mMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) &&
			(mMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	//没有找到可用内存类型
	return -1;
}

void RHIDevice::createInstance() {
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
    appInfo.pApplicationName = "GearEngine";
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

    CHECK_VKRESULT(vkCreateInstance(&instanceInfo, nullptr, &mInstance));
}

void RHIDevice::pickGPU() {
    uint32_t gpuCount = 0;
    if (vkEnumeratePhysicalDevices(mInstance, &gpuCount, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to find mGpu!");
        return ;
    }

    if (gpuCount == 0)
    {
        throw std::runtime_error("failed to find mGpu!");
        return ;
    }

    std::vector<VkPhysicalDevice> gpus(gpuCount);
    if (vkEnumeratePhysicalDevices(mInstance, &gpuCount, gpus.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to find mGpu!");
        return ;
    }

    for (auto &g : gpus)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(g, &props);
        printf("Found Vulkan GPU: %s\n", props.deviceName);
        printf("    API: %u.%u.%u\n",
             VK_VERSION_MAJOR(props.apiVersion),
             VK_VERSION_MINOR(props.apiVersion),
             VK_VERSION_PATCH(props.apiVersion));
        printf("    Driver: %u.%u.%u\n",
             VK_VERSION_MAJOR(props.driverVersion),
             VK_VERSION_MINOR(props.driverVersion),
             VK_VERSION_PATCH(props.driverVersion));
    }

    //默认选择第一块显卡
    mGPU = gpus.front();
}

void RHIDevice::setupDebugCallback() {
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