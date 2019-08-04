#ifndef RHI_QUEUE_H
#define RHI_QUEUE_H
#include "RHIDefine.h"

class RHIDevice;

class RHIQueue
{
public:
	RHIQueue(RHIDevice* device);
	~RHIQueue();

private:
	RHIDevice* mDevice;
	uint32_t mFamilyIndex;
};

#endif
