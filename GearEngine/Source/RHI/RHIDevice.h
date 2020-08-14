#ifndef RHI_DEVICE_H
#define RHI_DEVICE_H
#include "RHIDefine.h"

class RHIFence;
class RHISemaphore;
class RHIQueue;
class RHICommandBufferPool;

class RHIDevice
{
public:
	RHIDevice();
	~RHIDevice();
	VkDevice getDevice() { return mDevice; }
	VkPhysicalDevice getGPU() { return mGPU; }
    VkInstance getInstance() { return mInstance; }
    VkDescriptorPool getDescriptorPool(){ return mDescriptorPool; }
	RHIQueue* getGraphicsQueue() { return mGraphicsQueue; }
	uint32_t findMemoryType(const uint32_t &typeFilter, const VkMemoryPropertyFlags &properties);
    RHICommandBufferPool* getGraphicsCommandPool() { return mGraphicsCommandPool; }
private:
    void createInstance();
    void pickGPU();
    void setupDebugCallback();
private:
    VkInstance mInstance;
	VkPhysicalDevice mGPU;
    VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
	VkDevice mDevice;
	VkPhysicalDeviceProperties mDeviceProperties;
	VkPhysicalDeviceFeatures mDeviceFeatures;
	VkPhysicalDeviceMemoryProperties mMemoryProperties;
    VkDescriptorPool mDescriptorPool;
    RHICommandBufferPool* mGraphicsCommandPool;
	RHIQueue* mGraphicsQueue;
	RHIQueue* mComputeQueue;
	RHIQueue* mTransferQueue;
};

#endif