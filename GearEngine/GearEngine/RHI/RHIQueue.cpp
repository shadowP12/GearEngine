#include "RHIQueue.h"
#include "RHIDevice.h"

RHIQueue::RHIQueue(RHIDevice* device, VkQueue queue, uint32_t familyIndex)
	:mDevice(device), mQueue(queue), mFamilyIndex(familyIndex)
{
}

RHIQueue::~RHIQueue()
{

}