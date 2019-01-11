#ifndef MATERIAL_IMPORTER_H
#define MATERIAL_IMPORTER_H
#include "../Material/Material.h"
#include "../Resource/MaterialManager.h"
#include "../Resource/TextureManager.h"
#include "../ThirdParty/json.hpp"
#include "../RenderAPI/ShaderModule.h"
#include "../Renderer/Renderer.h"

class MaterialImporter
{
public:
	MaterialImporter();
	~MaterialImporter();
	void load(std::string name)
	{
		std::shared_ptr<Material> mat = std::shared_ptr<Material>(new Material());
		mat->setName(name);
		std::string path = "D:/GearEngine/GearEngine/Resource/Materials/" + name;//
		nlohmann::json json;
		std::ifstream file(path.c_str());
		if (!file.is_open()) 
		{
			throw std::runtime_error("Can't load file");
		}
		file >> json;
		auto& Textures = json["Textures"];
		auto& Shaders = json["Shader"];
		std::string vName = Shaders["vert"];
		std::string fName = Shaders["frag"];
		std::string vPath = "D:/GearEngine/GearEngine/Resource/Shaders/" + vName;
		std::string fPath = "D:/GearEngine/GearEngine/Resource/Shaders/" + fName;

		ShaderModule vertShader;
		ShaderModule fragShader;
		vertShader.load(vPath.data());
		fragShader.load(fPath.data());

		std::shared_ptr<ProgramInfo> vInfo = vertShader.getInfo();
		std::shared_ptr<ProgramInfo> fInfo = fragShader.getInfo();

		//DescriptorPool 
		VkDescriptorPool pool;
		std::vector<VkDescriptorPoolSize> poolSizes;

		VkDescriptorPoolSize uniformPoolSize = {};
		uniformPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformPoolSize.descriptorCount = vInfo->blockBuffers.size() + fInfo->blockBuffers.size();
		poolSizes.push_back(uniformPoolSize);

		VkDescriptorPoolSize samplerPoolSize = {};
		samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerPoolSize.descriptorCount = vInfo->sampler2Ds.size() + fInfo->sampler2Ds.size();
		poolSizes.push_back(samplerPoolSize);

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 1;//hard code : only one set

		if (vkCreateDescriptorPool(VulkanContext::instance().getDevice(), &poolInfo, nullptr, &pool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}
		mat->mPool = pool;

		//set up DescriptorSetLayout
		VkDescriptorSetLayout descriptorSetLayout;
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;

		for (uint32_t i = 0; i < vInfo->blockBuffers.size(); i++)
		{
			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.binding = vInfo->blockBuffers[i].binding;
			layoutBinding.descriptorCount = 1;
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			setLayoutBindings.push_back(layoutBinding);
		}
		for (uint32_t i = 0; i < vInfo->sampler2Ds.size(); i++)
		{
			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.binding = vInfo->sampler2Ds[i].binding;
			layoutBinding.descriptorCount = 1;
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			setLayoutBindings.push_back(layoutBinding);
		}
		for (uint32_t i = 0; i < fInfo->blockBuffers.size(); i++)
		{
			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.binding = fInfo->blockBuffers[i].binding;
			layoutBinding.descriptorCount = 1;
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			setLayoutBindings.push_back(layoutBinding);
		}
		for (uint32_t i = 0; i < fInfo->sampler2Ds.size(); i++)
		{
			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.binding = fInfo->sampler2Ds[i].binding;
			layoutBinding.descriptorCount = 1;
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			setLayoutBindings.push_back(layoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = setLayoutBindings.size();
		layoutInfo.pBindings = setLayoutBindings.data();

		if (vkCreateDescriptorSetLayout(VulkanContext::instance().getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
		mat->mDescriptorSetLayout = descriptorSetLayout;

		//DescriptorSet
		VkDescriptorSet set;
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;

		if (vkAllocateDescriptorSets(VulkanContext::instance().getDevice(), &allocInfo, &set) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to allocate descriptor set!");
		}

		mat->mDescriptorSet = set;

		std::vector<VkWriteDescriptorSet> descriptorWrites;
		for (uint32_t i = 0; i < vInfo->blockBuffers.size(); i++)
		{
			std::shared_ptr<UniformBuffer> ub = std::shared_ptr<UniformBuffer>(new UniformBuffer(vInfo->blockBuffers[i].size));
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = ub->getBuffer()->getBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = vInfo->blockBuffers[i].size;
			
			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = set;
			descriptorWrite.dstBinding = vInfo->blockBuffers[i].binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			descriptorWrites.push_back(descriptorWrite);
			//set up material
			mat->mBlockBuffers.push_back(vInfo->blockBuffers[i]);
			mat->mUniforms[vInfo->blockBuffers[i].name] = ub;
		}
		for (uint32_t i = 0; i < vInfo->sampler2Ds.size(); i++)
		{
			for (uint32_t j = 0; j < Textures.size(); j++)
			{
				if (Textures[j].find(vInfo->sampler2Ds[i].name) != Textures[i].end())
				{
					std::string texName = Textures[i][vInfo->sampler2Ds[i].name];
					std::shared_ptr<Texture> tex = std::dynamic_pointer_cast<Texture>(TextureManager::instance().getRes(texName));

					VkDescriptorImageInfo textureInfo;
					textureInfo.imageView = tex->getView();
					textureInfo.sampler = tex->getSampler();
					textureInfo.imageLayout = tex->getLayout();

					VkWriteDescriptorSet descriptorWrite = {};
					descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrite.dstSet = set;
					descriptorWrite.dstBinding = vInfo->sampler2Ds[i].binding;
					descriptorWrite.dstArrayElement = 0;
					descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorWrite.descriptorCount = 1;
					descriptorWrite.pImageInfo = &textureInfo;

					descriptorWrites.push_back(descriptorWrite);
					//set up material
					mat->mTextureBinding[vInfo->sampler2Ds[i].name] = vInfo->sampler2Ds[i].binding;
					mat->mTexture[vInfo->sampler2Ds[i].name] = tex;
				}
			}
		}
		for (uint32_t i = 0; i < fInfo->blockBuffers.size(); i++)
		{
			std::shared_ptr<UniformBuffer> ub = std::shared_ptr<UniformBuffer>(new UniformBuffer(fInfo->blockBuffers[i].size));
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = ub->getBuffer()->getBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = fInfo->blockBuffers[i].size;

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = set;
			descriptorWrite.dstBinding = fInfo->blockBuffers[i].binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			//set up material
			mat->mBlockBuffers.push_back(fInfo->blockBuffers[i]);
			mat->mUniforms[fInfo->blockBuffers[i].name] = ub;
		}
		for (uint32_t i = 0; i < fInfo->sampler2Ds.size(); i++)
		{
			for (uint32_t j = 0; j < Textures.size(); j++)
			{
				if (Textures[j].find(fInfo->sampler2Ds[i].name) != Textures[i].end())
				{
					std::string texName = Textures[i][fInfo->sampler2Ds[i].name];
					std::shared_ptr<Texture> tex = std::dynamic_pointer_cast<Texture>(TextureManager::instance().getRes(texName));

					VkDescriptorImageInfo textureInfo;
					textureInfo.imageView = tex->getView();
					textureInfo.sampler = tex->getSampler();
					textureInfo.imageLayout = tex->getLayout();

					VkWriteDescriptorSet descriptorWrite = {};
					descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrite.dstSet = set;
					descriptorWrite.dstBinding = fInfo->sampler2Ds[i].binding;
					descriptorWrite.dstArrayElement = 0;
					descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorWrite.descriptorCount = 1;
					descriptorWrite.pImageInfo = &textureInfo;

					descriptorWrites.push_back(descriptorWrite);
					//set up material
					mat->mTextureBinding[fInfo->sampler2Ds[i].name] = fInfo->sampler2Ds[i].binding;
					mat->mTexture[fInfo->sampler2Ds[i].name] = tex;
				}
			}
		}
		vkUpdateDescriptorSets(VulkanContext::instance().getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

		//seting PipelineLayout
		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShader.getShaderModule();
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShader.getShaderModule();
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		//dynamic state
		VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pNext = NULL;
		dynamicState.pDynamicStates = dynamicStateEnables;
		dynamicState.dynamicStateCount = 0;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = vInfo->stageInputsSize;
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		for (uint32_t i = 0; i < vInfo->stageInputs.size(); i++)
		{
			VkVertexInputAttributeDescription attributeDescription = {};
			attributeDescription.binding = 0;
			attributeDescription.location = i;
			attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription.offset = vInfo->stageInputs[i].offset;
			attributeDescriptions.push_back(attributeDescription);
		}

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = NULL;
		viewportState.flags = 0;
		viewportState.viewportCount = 1;
		dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
		viewportState.scissorCount = 1;
		dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
		viewportState.pScissors = NULL;
		viewportState.pViewports = NULL;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;


		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineLayout pipelineLayout;
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		if (vkCreatePipelineLayout(VulkanContext::instance().getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
		mat->mPipelineLayout = pipelineLayout;

		VkPipeline pipeline;
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = Renderer::instance().getRenderPass()->getRenderPass();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(VulkanContext::instance().getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline!");
		}
		mat->mPipeline = pipeline;
		MaterialManager::instance().addRes(mat);
	}
private:

};

MaterialImporter::MaterialImporter()
{
}

MaterialImporter::~MaterialImporter()
{
}
#endif
