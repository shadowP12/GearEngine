#ifndef RHI_DEVICE_H
#define RHI_DEVICE_H
#include "RHIDefine.h"
#include <vector>
class RHIDevice
{
public:
	RHIDevice(VkPhysicalDevice gpu);
	~RHIDevice();
	VkDevice getDevice() { return mDevice; }
	VkPhysicalDevice getPhyDevice() { return mGPU; }
	uint32_t getGraphicsFamily() { return mGraphicsFamily; }
	uint32_t findMemoryType(const uint32_t &typeFilter, const VkMemoryPropertyFlags &properties);
private:
	void createCommandPool();
private:
	friend class RHI;
	VkPhysicalDevice mGPU;
	VkDevice mDevice;
	VkPhysicalDeviceProperties mDeviceProperties;
	VkPhysicalDeviceFeatures mDeviceFeatures;
	VkPhysicalDeviceMemoryProperties mMemoryProperties;
	VkCommandPool mGraphicsCommandPool;
	VkCommandPool mComputeCommandPool;
	VkCommandPool mTransferCommandPool;
	uint32_t mGraphicsFamily;
	uint32_t mComputeFamily;
	uint32_t mTransferFamily;
	VkQueue mGraphicsQueue;
	VkQueue mComputeQueue;
	VkQueue mTransferQueue;
};

#endif

