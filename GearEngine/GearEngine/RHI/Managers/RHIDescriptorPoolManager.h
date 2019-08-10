#ifndef RHI_DESCRIPTOR_POOL_MANAGER_H
#define RHI_DESCRIPTOR_POOL_MANAGER_H
#include "RHI/RHIDefine.h"

class RHIDevice;

class RHIDescriptorPoolManager
{
public:
	RHIDescriptorPoolManager(RHIDevice* device);
	~RHIDescriptorPoolManager();

private:
	RHIDevice* mDevice;
};

#endif
