#include "VulkanContext.h"
#include <assert.h>
VulkanContext::VulkanContext(GLFWwindow* window)
{
	mValidationLayers.push_back("VK_LAYER_LUNARG_standard_validation");
	mDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	createInstance();
	createSurface(window);
	pickPhysicalDevice();
	createLogicalDevice();
	mManager = new VulkanResourceManager();
}

VulkanContext::~VulkanContext()
{
	delete mManager;
}

uint32_t VulkanContext::findMemoryType(const uint32_t & typeFilter, const VkMemoryPropertyFlags & properties)
{
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &physicalDeviceMemoryProperties);

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

void VulkanContext::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport()) 
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "GearEngine_0.1";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "GearEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (enableValidationLayers) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
		createInfo.ppEnabledLayerNames = mValidationLayers.data();
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create instance!");
	}
}

void VulkanContext::createSurface(GLFWwindow* window)
{
	auto res = glfwCreateWindowSurface(mInstance, window, nullptr, &mSurface);
	if (res != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

bool VulkanContext::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : mValidationLayers) 
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) 
		{
			if (strcmp(layerName, layerProperties.layerName) == 0) 
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound) 
		{
			return false;
		}
	}

	return true;
}

std::vector<const char*> VulkanContext::getRequiredExtensions()
{
	std::vector<const char*> extensions;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++) 
	{
		extensions.push_back(glfwExtensions[i]);
	}

	if (enableValidationLayers) 
	{
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

void VulkanContext::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

	if (deviceCount == 0) 
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

	for (const auto& device : devices) 
	{
		if (isDeviceSuitable(device)) 
		{
			mPhysicalDevice = device;
			break;
		}
	}

	if (mPhysicalDevice == VK_NULL_HANDLE) 
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void VulkanContext::createLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
	float queuePriorities[] = { 0.0f };

	if (mSupportedQueues & VK_QUEUE_GRAPHICS_BIT)
	{
		VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
		graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		graphicsQueueCreateInfo.queueFamilyIndex = mGraphicsFamily;
		graphicsQueueCreateInfo.queueCount = 1;
		graphicsQueueCreateInfo.pQueuePriorities = queuePriorities;
		queueCreateInfos.emplace_back(graphicsQueueCreateInfo);
	}
	else
	{
		mGraphicsFamily = VK_NULL_HANDLE;
	}

	if (mSupportedQueues & VK_QUEUE_COMPUTE_BIT && mComputeFamily != mGraphicsFamily)
	{
		VkDeviceQueueCreateInfo computeQueueCreateInfo = {};
		computeQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		computeQueueCreateInfo.queueFamilyIndex = mComputeFamily;
		computeQueueCreateInfo.queueCount = 1;
		computeQueueCreateInfo.pQueuePriorities = queuePriorities;
		queueCreateInfos.emplace_back(computeQueueCreateInfo);
	}
	else
	{
		mComputeFamily = mGraphicsFamily;
	}

	if (mSupportedQueues & VK_QUEUE_TRANSFER_BIT && mTransferFamily != mGraphicsFamily && mTransferFamily != mComputeFamily)
	{
		VkDeviceQueueCreateInfo transferQueueCreateInfo = {};
		transferQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		transferQueueCreateInfo.queueFamilyIndex = mTransferFamily;
		transferQueueCreateInfo.queueCount = 1;
		transferQueueCreateInfo.pQueuePriorities = queuePriorities;
		queueCreateInfos.emplace_back(transferQueueCreateInfo);
	}
	else
	{
		mTransferFamily = mGraphicsFamily;
	}

	//todo
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.sampleRateShading = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;
	deviceFeatures.wideLines = VK_TRUE;
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.vertexPipelineStoresAndAtomics = VK_TRUE;
	deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;
	deviceFeatures.shaderStorageImageExtendedFormats = VK_TRUE;
	deviceFeatures.shaderStorageImageWriteWithoutFormat = VK_TRUE;
	deviceFeatures.shaderClipDistance = VK_TRUE;
	deviceFeatures.shaderCullDistance = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
	deviceCreateInfo.ppEnabledLayerNames = mValidationLayers.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(mDeviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = mDeviceExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	if (vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(mDevice, mGraphicsFamily, 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, mPresentFamily, 0, &mPresentQueue);
	vkGetDeviceQueue(mDevice, mComputeFamily, 0, &mComputeQueue);
	vkGetDeviceQueue(mDevice, mTransferFamily, 0, &mTransferQueue);
}

bool VulkanContext::isDeviceSuitable(VkPhysicalDevice device)
{
	return findQueueFamilies(device);
}

bool VulkanContext::findQueueFamilies(VkPhysicalDevice device)
{
	uint32_t deviceQueueFamilyPropertyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &deviceQueueFamilyPropertyCount, nullptr);
	std::vector<VkQueueFamilyProperties> deviceQueueFamilyProperties(deviceQueueFamilyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &deviceQueueFamilyPropertyCount, deviceQueueFamilyProperties.data());

	uint32_t graphicsFamily = -1;
	uint32_t presentFamily = -1;
	uint32_t computeFamily = -1;
	uint32_t transferFamily = -1;

	for (auto i = 0; i < deviceQueueFamilyPropertyCount; i++)
	{
		// Check for graphics support.
		if (deviceQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsFamily = i;
			mGraphicsFamily = i;
			mSupportedQueues |= VK_QUEUE_GRAPHICS_BIT;
		}

		// Check for presentation support.
		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);

		if (deviceQueueFamilyProperties[i].queueCount > 0 && presentSupport)
		{
			presentFamily = i;
			mPresentFamily = i;
		}

		// Check for compute support.
		if (deviceQueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			computeFamily = i;
			mComputeFamily = i;
			mSupportedQueues |= VK_QUEUE_COMPUTE_BIT;
		}

		// Check for transfer support.
		if (deviceQueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			transferFamily = i;
			mTransferFamily = i;
			mSupportedQueues |= VK_QUEUE_TRANSFER_BIT;
		}

		if (graphicsFamily != -1 && presentFamily != -1 && computeFamily != -1 && transferFamily != -1)
		{
			break;
		}
	}

	if (graphicsFamily == -1)
	{
		return false;
	}
	return true;
}

