#include "RHIDevice.h"

RHIDevice::RHIDevice(VkPhysicalDevice gpu)
	:mGPU(gpu)
{
	vkGetPhysicalDeviceProperties(mGPU, &mDeviceProperties);
	vkGetPhysicalDeviceFeatures(mGPU, &mDeviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(mGPU, &mMemoryProperties);

	//Note:这里有可能会出现三个queue的Family相同的情况
	uint32_t numQueueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties(mGPU, &numQueueFamilies, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyProperties(numQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(mGPU, &numQueueFamilies, queueFamilyProperties.data());

	mGraphicsFamily = -1;
	mComputeFamily = -1;
	mTransferFamily = -1;

	for (uint32_t i = 0; i < (uint32_t)queueFamilyProperties.size(); i++)
	{
		if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
		{
			mComputeFamily = i;
			break;
		}
	}

	for (uint32_t i = 0; i < (uint32_t)queueFamilyProperties.size(); i++)
	{
		if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
			((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
			((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
		{
			mTransferFamily = i;
			break;
		}
	}

	for (uint32_t i = 0; i < (uint32_t)queueFamilyProperties.size(); i++)
	{
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			mGraphicsFamily = i;
			break;
		}
	}

	static const float graphicsQueuePrio = 0.5f;
	static const float computeQueuePrio = 1.0f;
	static const float transferQueuePrio = 1.0f;

	VkDeviceQueueCreateInfo queueInfo[3] = {};

	queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo[0].queueFamilyIndex = mGraphicsFamily;
	queueInfo[0].queueCount = queueFamilyProperties[mGraphicsFamily].queueCount;
	queueInfo[0].pQueuePriorities = &graphicsQueuePrio;

	queueInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo[1].queueFamilyIndex = mComputeFamily;
	queueInfo[1].queueCount = queueFamilyProperties[mComputeFamily].queueCount;
	queueInfo[1].pQueuePriorities = &computeQueuePrio;

	queueInfo[2].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo[2].queueFamilyIndex = mTransferFamily;
	queueInfo[2].queueCount = queueFamilyProperties[mTransferFamily].queueCount;
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
	deviceInfo.queueCreateInfoCount = 3;
	deviceInfo.pQueueCreateInfos = queueInfo;
	deviceInfo.pEnabledFeatures = &mDeviceFeatures;
	deviceInfo.enabledExtensionCount = numExtensions;
	deviceInfo.ppEnabledExtensionNames = extensions;
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = nullptr;

	if (vkCreateDevice(mGPU, &deviceInfo, nullptr, &mDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(mDevice, mGraphicsFamily, 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, mComputeFamily, 0, &mComputeQueue);
	vkGetDeviceQueue(mDevice, mTransferFamily, 0, &mTransferQueue);
}


RHIDevice::~RHIDevice()
{
	vkDestroyDevice(mDevice, nullptr);
}

void RHIDevice::createCommandPool()
{
	//图形命令池和计算命令池是可重复使用
	VkCommandPoolCreateInfo graphicsPoolInfo = {};
	graphicsPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	graphicsPoolInfo.queueFamilyIndex = mGraphicsFamily;
	graphicsPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(mDevice, &graphicsPoolInfo, nullptr, &mGraphicsCommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics command pool!");
	}

	VkCommandPoolCreateInfo computePoolInfo = {};
	computePoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	computePoolInfo.queueFamilyIndex = mComputeFamily;
	computePoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(mDevice, &computePoolInfo, nullptr, &mComputeCommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create compute command pool!");
	}
	VkCommandPoolCreateInfo transferPoolInfo = {};
	transferPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	transferPoolInfo.queueFamilyIndex = mTransferFamily;
	transferPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	if (vkCreateCommandPool(mDevice, &transferPoolInfo, nullptr, &mTransferCommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create transfer command pool!");
	}
}
