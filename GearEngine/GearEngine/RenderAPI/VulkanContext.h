#ifndef VULKAN_CONTEXT_H
#define VULKAN_CONTEXT_H
#include "VulkanPrerequisites.h"
#include "../Utility/Module.h"
#include <vector>
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class VulkanContext : public Module<VulkanContext>
{
public:
	VulkanContext();
	~VulkanContext();
	void createInstance();
private:
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool findQueueFamilies(VkPhysicalDevice device);
private:
	std::vector<const char*> mValidationLayers;
	std::vector<const char*> mDeviceExtensions;
	VkInstance mInstance;
	VkSurfaceKHR mSurface;
	VkDevice mDevice;
	VkPhysicalDevice mPhysicalDevice;
	VkQueueFlags mSupportedQueues;
	uint32_t mGraphicsFamily;
	uint32_t mPresentFamily;
	uint32_t mComputeFamily;
	uint32_t mTransferFamily;
	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;
	VkQueue mComputeQueue;
	VkQueue mTransferQueue;
};
#endif