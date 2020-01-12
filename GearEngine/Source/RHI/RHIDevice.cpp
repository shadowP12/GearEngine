#include "RHIDevice.h"

RHIDevice::RHIDevice(VkPhysicalDevice gpu)
	:mGPU(gpu)
{
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

	if (vkCreateDevice(mGPU, &deviceInfo, nullptr, &mDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	VkQueue graphicsQueue;
	VkQueue computeQueue;
	VkQueue transferQueue;

	vkGetDeviceQueue(mDevice, graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(mDevice, computeFamily, 0, &computeQueue);
	vkGetDeviceQueue(mDevice, transferFamily, 0, &transferQueue);

	mGraphicsQueue = new RHIQueue(this, graphicsQueue, graphicsFamily);
	mComputeQueue = new RHIQueue(this, computeQueue, computeFamily);
	mTransferQueue = new RHIQueue(this, transferQueue, transferFamily);

	//
	createCommandPool();

	//
	mDummyUniformBuffer = createUniformBuffer(16);
}

RHIDevice::~RHIDevice()
{
	SAFE_DELETE(mGraphicsCommandPool);
	SAFE_DELETE(mComputeCommandPool);
	SAFE_DELETE(mTransferCommandPool);
	SAFE_DELETE(mGraphicsQueue);
	SAFE_DELETE(mComputeQueue);
	SAFE_DELETE(mTransferQueue);
	SAFE_DELETE(mDummyUniformBuffer);
	vkDestroyDevice(mDevice, nullptr);
}

void RHIDevice::createCommandPool()
{
	mGraphicsCommandPool = new RHICommandBufferPool(this, mGraphicsQueue, true);
	mComputeCommandPool = new RHICommandBufferPool(this, mComputeQueue, true);
	mTransferCommandPool = new RHICommandBufferPool(this, mTransferQueue, false);
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

RHIBuffer * RHIDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags)
{
	RHIBuffer* res = new RHIBuffer(this, size, usageFlags, memoryPropertyFlags);
	return res;
}

RHICommandBuffer * RHIDevice::allocCommandBuffer(const CommandBufferType& type, bool primary)
{
	RHICommandBuffer* commandBuffer = nullptr;
	if (type == CommandBufferType::GRAPHICS)
	{
		commandBuffer = mGraphicsCommandPool->allocCommandBuffer(primary);
	}
	else if (type == CommandBufferType::COMPUTE)
	{
		commandBuffer = mComputeCommandPool->allocCommandBuffer(primary);
	}
	else
	{
		commandBuffer = mTransferCommandPool->allocCommandBuffer(primary);
	}
	return commandBuffer;
}

RHIProgram * RHIDevice::createProgram(const RHIProgramInfo & programInfo)
{
	RHIProgram* ret = new RHIProgram(this, programInfo);
	return ret;
}

RHIUniformBuffer* RHIDevice::createUniformBuffer(uint32_t size)
{
	RHIUniformBuffer* ret = new RHIUniformBuffer(this, size);
	return ret;
}

RHIVertexBuffer* RHIDevice::createVertexBuffer(uint32_t elementSize, uint32_t vertexCount)
{
	RHIVertexBuffer* ret = new RHIVertexBuffer(this, elementSize, vertexCount);
	return ret;
}

RHIIndexBuffer* RHIDevice::createIndexBuffer(uint32_t elementSize, uint32_t indexCount)
{
	RHIIndexBuffer* ret = new RHIIndexBuffer(this, elementSize, indexCount);
	return ret;
}

RHITransferBuffer* RHIDevice::createTransferBuffer(uint32_t size)
{
	RHITransferBuffer* ret = new RHITransferBuffer(this, size);
	return ret;
}

RHIRenderPass* RHIDevice::createRenderPass(const RHIRenderPassInfo& renderPassInfo)
{
	RHIRenderPass* ret = new RHIRenderPass(this, renderPassInfo);
	return ret;
}

RHIFramebuffer* RHIDevice::createFramebuffer(const RHIFramebufferInfo& framebufferInfo)
{
	RHIFramebuffer* ret = new RHIFramebuffer(this, framebufferInfo);
	return ret;
}

RHITexture* RHIDevice::createTexture(const RHITextureInfo& textureInfo)
{
	RHITexture* ret = new RHITexture(this, textureInfo);
	return ret;
}

RHITextureView* RHIDevice::createTextureView(const RHITextureViewInfo& viewInfo)
{
	RHITextureView* ret = new RHITextureView(this, viewInfo);
	return ret;
}

RHITextureView* RHIDevice::createTextureView(VkImageView view)
{
	RHITextureView* ret = new RHITextureView(this, view);
	return ret;
}