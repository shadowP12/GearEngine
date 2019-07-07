#ifndef RHI_H
#define RHI_H
#include "RHIDefine.h"
#include "RHIDevice.h"
#include "Utility/Module.h"
#include <vector>

class RHI : public Module<RHI>
{
public:
	RHI();
	~RHI();
	uint32_t findMemoryType(const uint32_t &typeFilter, const VkMemoryPropertyFlags &properties);
private:
	void createInstance();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createCommandPool();
	void setupDebugCallback();
private:
	VkInstance mInstance;
	VkPhysicalDevice mGPU;
	VkDebugReportCallbackEXT mDebugCallback;
};
#endif