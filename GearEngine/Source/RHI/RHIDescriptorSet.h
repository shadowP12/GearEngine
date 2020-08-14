#ifndef RHI_DESCRIPTOR_SET_H
#define RHI_DESCRIPTOR_SET_H
#include "RHIDefine.h"
#include <string>

class RHIDevice;
class RHIBuffer;
class RHITexture;
class RHISampler;

struct RHIDescriptorSetBinding
{
    uint32_t binding;
    uint32_t descriptorCount;
    DescriptorType type;
    ProgramType stage;
};

struct RHIDescriptorSetInfo
{
    uint32_t set;
    RHIDescriptorSetBinding bindings[8];
    uint32_t bindingCount;
};

class RHIDescriptorSet
{
public:
    RHIDescriptorSet(RHIDevice* device, RHIDescriptorSetInfo info);
	~RHIDescriptorSet();
    VkDescriptorSet getHandle() { return mSet; }
    VkDescriptorSetLayout getLayout() { return mLayout; }
	void updateBuffer(uint32_t binding, RHIBuffer* buffer, uint32_t size, uint32_t offset = 0);
    void updateTexture(uint32_t binding, RHITexture* texture, RHISampler* sampler = nullptr);
private:
	RHIDevice* mDevice;
    RHIDescriptorSetInfo mInfo;
    VkDescriptorSet mSet;
    VkDescriptorSetLayout mLayout;
};
#endif