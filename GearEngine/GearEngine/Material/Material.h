#ifndef MATERIAL_H
#define MATERIAL_H
#include "../Resource/Resource.h"
#include "../Texture/Texture.h"
#include "../RenderAPI/UniformBuffer.h"
#include "../Utility/ShaderTool/ShaderInfo.h"
#include <map>

class Material : public Resource
{
public:
	Material();
	~Material();

	void setUniform(std::string uniformName, std::string memberName, void* data)
	{
		for (uint32_t i = 0; i < mBlockBuffers.size(); i++)
		{
			if (uniformName == mBlockBuffers[i].name)
			{
				for (uint32_t j = 0; j < mBlockBuffers[i].members.size(); j++)
				{
					if (memberName == mBlockBuffers[i].members[j].name)
					{
						mUniforms[uniformName]->writeData(mBlockBuffers[i].members[j].offset, mBlockBuffers[i].members[j].size, data);
						return;
					}
				}
			}
		}
	}
	void setTexture(std::string name,std::shared_ptr<Texture> tex) 
	{
		std::map<std::string, std::shared_ptr<Texture>>::iterator nameIt = mTexture.find(name);
		if (nameIt != mTexture.end())
		{
			nameIt->second = tex;
			uint32_t set = mTextureSet[name].first;
			uint32_t binding = mTextureSet[name].second;

			VkDescriptorImageInfo textureDescriptor;
			textureDescriptor.imageView = tex->getView();			
			textureDescriptor.sampler = tex->getSampler();
			textureDescriptor.imageLayout = tex->getLayout();

			VkWriteDescriptorSet writeDescriptorSet{};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = mDescriptorSet[set];
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSet.dstBinding = binding;
			writeDescriptorSet.pImageInfo = &textureDescriptor;
			writeDescriptorSet.descriptorCount = 1;

			vkUpdateDescriptorSets(VulkanContext::instance().getDevice(), 1, &writeDescriptorSet, 0, nullptr);
		}
	}
private:
	std::map<std::string, std::shared_ptr<UniformBuffer>> mUniforms;
	std::map<std::string, std::shared_ptr<Texture>> mTexture;
	std::map<std::string, std::pair<uint32_t, uint32_t>> mTextureSet;
	std::vector<VkDescriptorSet> mDescriptorSet;
	std::vector<BlockBuffer> mBlockBuffers;
};

Material::Material()
{
}

Material::~Material()
{
}
#endif
