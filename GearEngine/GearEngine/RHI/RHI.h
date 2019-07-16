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
	VkInstance getInstance() { return mInstance; }
	RHIDevice* getDevice() { return mActiceDevice; }
private:
	void createInstance();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void setupDebugCallback();
private:
	VkInstance mInstance;
	VkPhysicalDevice mGPU;
	RHIDevice* mActiceDevice;
	VkDebugReportCallbackEXT mDebugCallback;
};
#endif