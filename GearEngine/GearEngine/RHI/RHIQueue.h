#ifndef RHI_QUEUE_H
#define RHI_QUEUE_H
#include "RHIDefine.h"

class RHIDevice;

class RHIQueue
{
public:
	RHIQueue(RHIDevice* device, VkQueue queue, uint32_t familyIndex);
	~RHIQueue();
	uint32_t getFamilyIndex() { return mFamilyIndex; }
private:
	RHIDevice* mDevice;
	VkQueue mQueue;
	uint32_t mFamilyIndex;
};

#endif
