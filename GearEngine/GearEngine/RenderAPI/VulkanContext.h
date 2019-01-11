#ifndef VULKAN_CONTEXT_H
#define VULKAN_CONTEXT_H
#include "VulkanPrerequisites.h"
#include "VulkanResource.h"
#include "../Utility/Module.h"
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

inline VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback) 
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) 
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else 
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

inline void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) 
	{
		func(instance, callback, pAllocator);
	}
}

class VulkanContext : public Module<VulkanContext>
{
public:
	VulkanContext(GLFWwindow* window);
	~VulkanContext();
	VkDevice getDevice() { return mDevice; }
	VkPhysicalDevice getPhyDevice() { return mPhysicalDevice; }
	VkSurfaceKHR getSurface() { return mSurface; }
	VulkanResourceManager* getResourceManager() { return mManager; }
	VkCommandPool getCommandPool() { return mCommandPool; }
	VkQueue getGraphicsQueue() { return mGraphicsQueue; }
	VkQueue getComputeQueue() { return mComputeQueue; }
	VkQueue getPresentQueue() { return mPresentQueue; }
	uint32_t getGraphicsFamily() { return mGraphicsFamily; }
	uint32_t getPresentFamily() { return mPresentFamily; }
	uint32_t findMemoryType(const uint32_t &typeFilter, const VkMemoryPropertyFlags &properties);
private:
	void createInstance();
	void createSurface(GLFWwindow* window);
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createCommandPool();
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool findQueueFamilies(VkPhysicalDevice device);
	void setupDebugCallback();
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
private:
	VulkanResourceManager* mManager;
	std::vector<const char*> mValidationLayers;
	std::vector<const char*> mDeviceExtensions;
	VkDebugUtilsMessengerEXT mCallback;
	VkInstance mInstance;
	VkSurfaceKHR mSurface;
	VkDevice mDevice;
	VkPhysicalDevice mPhysicalDevice;
	VkCommandPool mCommandPool;
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