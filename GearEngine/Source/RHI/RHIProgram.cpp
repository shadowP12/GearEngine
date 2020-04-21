#include "RHIProgram.h"
#include "RHIDevice.h"
#include "RHIBuffers.h"
#include "RHITexture.h"
#include "RHITextureView.h"
#include "Managers/RHIProgramManager.h"

RHIProgram::RHIProgram(RHIDevice* device, RHIProgramManager* mgr, const RHIProgramInfo& info)
	:mDevice(device), mProgramMgr(mgr), mSource(info.source), mEntryPoint(info.entryPoint), mType(info.type)
{
    mIsCompiled = false;
}

RHIProgram::~RHIProgram()
{
    mProgramMgr->deleteProgram(this);
	vkDestroyShaderModule(mDevice->getDevice(), mModule, nullptr);
}

void RHIProgram::compile()
{
	mProgramMgr->compile(this);
	VkShaderModuleCreateInfo moduleInfo;
	moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo.pNext = nullptr;
	moduleInfo.flags = 0;
	moduleInfo.codeSize = mBytecode.size() * sizeof(uint32_t);
	moduleInfo.pCode = mBytecode.data();

	if (vkCreateShaderModule(mDevice->getDevice(), &moduleInfo, nullptr, &mModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

    mIsCompiled = true;

	// stage1
	std::map<uint32_t, bool> setInfos;
    for (int i = 0; i < mUniformBufferInfos.size(); ++i)
    {
        setInfos[mUniformBufferInfos[i].set] = true;
    }
    for (int i = 0; i < mSampleImageInfos.size(); ++i)
    {
        setInfos[mSampleImageInfos[i].set] = true;
    }

    // stage2
    std::map<uint32_t, VkDescriptorSetLayoutCreateInfo> layoutCreateInfos;
    for (auto& setInfo : setInfos)
    {
        uint32_t set = setInfo.first;
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
        setLayoutBindings.clear();

        for (int i = 0; i < mUniformBufferInfos.size(); ++i)
        {
            if(mUniformBufferInfos[i].set == set)
            {
                VkDescriptorSetLayoutBinding layoutBinding = {};
                layoutBinding.binding = mUniformBufferInfos[i].binding;
                layoutBinding.descriptorCount = 1;
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                layoutBinding.pImmutableSamplers = nullptr;
                if(mType == RHIProgramType::Vertex)
                {
                    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                }
                else if(mType == RHIProgramType::Fragment)
                {
                    layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                }
                else
                {
                    // error
                }

                setLayoutBindings.push_back(layoutBinding);
            }
        }
        for (int i = 0; i < mSampleImageInfos.size(); ++i)
        {
            if(mSampleImageInfos[i].set == set)
            {
                VkDescriptorSetLayoutBinding layoutBinding = {};
                layoutBinding.binding = mSampleImageInfos[i].binding;
                layoutBinding.descriptorCount = 1;
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                layoutBinding.pImmutableSamplers = nullptr;
                if(mType == RHIProgramType::Vertex)
                {
                    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                }
                else if(mType == RHIProgramType::Fragment)
                {
                    layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                }
                else
                {
                    // error
                }
                setLayoutBindings.push_back(layoutBinding);
            }
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = setLayoutBindings.size();
        layoutInfo.pBindings = setLayoutBindings.data();
        layoutCreateInfos[set] = layoutInfo;
    }

    // stage3
    std::map<uint32_t , VkDescriptorSetLayout> setLayouts;
    for (auto& layoutCreateInfo : layoutCreateInfos)
    {
        VkDescriptorSetLayout setLayout;
        if (vkCreateDescriptorSetLayout(mDevice->getDevice(), &layoutCreateInfo.second, nullptr, &setLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
        setLayouts[layoutCreateInfo.first] = setLayout;
    }

    // stage4
    for (auto& setLayout : setLayouts)
    {
        VkDescriptorSetLayout layouts[] = { setLayout.second };
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mDevice->getDescriptorPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts;

        VkDescriptorSet descriptorSet;
        if (vkAllocateDescriptorSets(mDevice->getDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor set!");
        }
        mDescriptorSets[setLayout.first] = descriptorSet;
    }
}

void RHIProgram::setUniformBuffer(std::string name, RHIUniformBuffer* ub)
{
    uint32_t flag = 0;
    uint32_t set = -1;
    uint32_t binding = -1;
    uint32_t size = 0;

    for (int i = 0; i < mUniformBufferInfos.size(); ++i)
    {
        if(mUniformBufferInfos[i].name == name)
        {
            flag = 1;
            set = mUniformBufferInfos[i].set;
            binding = mUniformBufferInfos[i].binding;
            size = mUniformBufferInfos[i].size;
            break;
        }
    }

    if (flag == 0)
        return;

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = ub->getBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = size;

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = set;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(mDevice->getDevice(), 1, &descriptorWrite, 0, nullptr);
}
