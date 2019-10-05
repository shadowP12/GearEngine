#ifndef RHI_UTILITY_H
#define RHI_UTILITY_H
#include "RHIDefine.h"

VkAccessFlags getAccessMask(VkImageLayout layout);

VkPipelineStageFlags getStageFlags(VkImageLayout layout);

#endif