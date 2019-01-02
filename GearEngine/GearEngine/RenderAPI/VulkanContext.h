#ifndef VULKAN_CONTEXT_H
#define VULKAN_CONTEXT_H
#include "VulkanPrerequisites.h"
#include "VulkanResource.h"
#include "../Utility/Module.h"
#include <vector>
#include <glfw3.h>
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class VulkanContext : public Module<VulkanContext>
{
public:
	VulkanContext(GLFWwindow* window);
	~VulkanContext();
	VkDevice getDevice() { return mDevice; }
	VkPhysicalDevice getPhyDevice() { return mPhysicalDevice; }
	VkSurfaceKHR getSurface() { return mSurface; }
	VulkanResourceManager* getResourceManager() { return mManager; }
	uint32_t findMemoryType(const uint32_t &typeFilter, const VkMemoryPropertyFlags &properties);
private:
	void createInstance();
	void createSurface(GLFWwindow* window);
	void pickPhysicalDevice();
	void createLogicalDevice();
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool findQueueFamilies(VkPhysicalDevice device);
private:
	VulkanResourceManager* mManager;
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