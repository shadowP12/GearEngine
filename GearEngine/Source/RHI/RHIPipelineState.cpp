#include "RHIPipelineState.h"
#include "RHIDevice.h"
#include "Math/GMath.h"
#include <array>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
};

RHIGraphicsPipelineState::VariantKey::VariantKey(uint32_t inRenderpassID)
	:renderpassID(inRenderpassID)
{
}

size_t RHIGraphicsPipelineState::VariantKey::HashFunction::operator()(const VariantKey& v) const
{
	size_t hash = 0;
	Hash(hash, v.renderpassID);

	return hash;
}

bool RHIGraphicsPipelineState::VariantKey::EqualFunction::operator()(
	const VariantKey& lhs, const VariantKey& rhs) const
{
	if (lhs.renderpassID != rhs.renderpassID)
		return false;

	return true;
}

RHIGraphicsPipelineState::RHIGraphicsPipelineState(RHIDevice* device, const RHIPipelineStateInfo& info)
	:mDevice(device), mVertexProgram(info.vertexProgram), mFragmentProgram(info.fragmentProgram)
{
	// note:vertexProgram和fragmentProgram的sets不应该重复
	for (uint32_t i = 0; i < mVertexProgram->mParamInfo.sets.size(); i++)
	{
		mSets.push_back(mVertexProgram->mParamInfo.sets[i]);
	}
	for (uint32_t i = 0; i < mFragmentProgram->mParamInfo.sets.size(); i++)
	{
		mSets.push_back(mFragmentProgram->mParamInfo.sets[i]);
	}
	// 创建DescriptorPool
	VkDescriptorPoolSize poolSizes[2];
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; 
	poolSizes[0].descriptorCount = mVertexProgram->mParamInfo.samplers.size() + mFragmentProgram->mParamInfo.samplers.size();
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = mVertexProgram->mParamInfo.paramBlocks.size() + mFragmentProgram->mParamInfo.paramBlocks.size();

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);;
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = mSets.size();

	if (vkCreateDescriptorPool(mDevice->getDevice(), &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}

	// 创建Descriptor Layouts
	std::map<uint32_t, VkDescriptorSetLayoutCreateInfo> layoutCreateInfos;
	
	for (uint32_t i = 0; i < mSets.size(); i++)
	{
		uint32_t set = mSets[i];
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;

		for (auto& entry : mVertexProgram->mParamInfo.paramBlocks)
		{
			if (entry.second.set == set)
			{
				VkDescriptorSetLayoutBinding layoutBinding = {};
				layoutBinding.binding = entry.second.slot;
				layoutBinding.descriptorCount = 1;
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				layoutBinding.pImmutableSamplers = nullptr;
				layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				setLayoutBindings.push_back(layoutBinding);
			}
		}

		for (auto& entry : mVertexProgram->mParamInfo.samplers)
		{
			if (entry.second.set == set)
			{
				VkDescriptorSetLayoutBinding layoutBinding = {};
				layoutBinding.binding = entry.second.slot;
				layoutBinding.descriptorCount = 1;
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				layoutBinding.pImmutableSamplers = nullptr;
				layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				setLayoutBindings.push_back(layoutBinding);
			}
		}

		for (auto& entry : mFragmentProgram->mParamInfo.paramBlocks)
		{
			if (entry.second.set == set)
			{
				VkDescriptorSetLayoutBinding layoutBinding = {};
				layoutBinding.binding = entry.second.slot;
				layoutBinding.descriptorCount = 1;
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				layoutBinding.pImmutableSamplers = nullptr;
				layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				setLayoutBindings.push_back(layoutBinding);
			}
		}
		for (auto& entry : mFragmentProgram->mParamInfo.paramBlocks)
		{
			if (entry.second.set == set)
			{
				VkDescriptorSetLayoutBinding layoutBinding = {};
				layoutBinding.binding = entry.second.slot;
				layoutBinding.descriptorCount = 1;
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				layoutBinding.pImmutableSamplers = nullptr;
				layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				setLayoutBindings.push_back(layoutBinding);
			}
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = setLayoutBindings.size();
		layoutInfo.pBindings = setLayoutBindings.data();
		
		layoutCreateInfos[set] = layoutInfo;
	}

	std::vector<VkDescriptorSetLayout> setLayouts;
	for (auto& entry : layoutCreateInfos)
	{
		VkDescriptorSetLayout setLayout;
		if (vkCreateDescriptorSetLayout(mDevice->getDevice(), &entry.second, nullptr, &setLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
		mDescriptorSetLayouts[entry.first] = setLayout;
		setLayouts.push_back(setLayout);
	}

	// 创建desc set
	for (auto& entry : mDescriptorSetLayouts)
	{
		VkDescriptorSetLayout layouts[] = { entry.second };
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts;

		VkDescriptorSet descriptorSet;
		if (vkAllocateDescriptorSets(mDevice->getDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to allocate descriptor set!");
		}
		mDescriptorSets[entry.first] = descriptorSet;
		mDescriptorSets2.push_back(descriptorSet);
	}

	// 创建管线布局
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = setLayouts.size();
	pipelineLayoutInfo.pSetLayouts = setLayouts.data();

	if (vkCreatePipelineLayout(mDevice->getDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// 创建program parm list
	mParamList = new RHIProgramParamList(mDevice, mDescriptorSets,mVertexProgram->mParamInfo, mFragmentProgram->mParamInfo);
}

RHIGraphicsPipelineState::~RHIGraphicsPipelineState()
{
	for (auto& entry : mDescriptorSetLayouts)
	{
		vkDestroyDescriptorSetLayout(mDevice->getDevice(), entry.second, nullptr);
	}

	for (auto& entry : mDescriptorSets)
	{
		VkResult result = vkFreeDescriptorSets(mDevice->getDevice(), mDescriptorPool, 1, &entry.second);
		assert(result == VK_SUCCESS);
	}

	vkDestroyPipelineLayout(mDevice->getDevice(), mPipelineLayout, nullptr);
	vkDestroyDescriptorPool(mDevice->getDevice(), mDescriptorPool, nullptr);

	if (mParamList)
		delete mParamList;
}

VkPipeline RHIGraphicsPipelineState::getPipeline(RHIRenderPass * renderPass)
{
	VariantKey key(renderPass->getID());
	auto iterFind = mPipelines.find(key);
	if (iterFind != mPipelines.end())
		return iterFind->second;

	VkRenderPass newVariant = createVariant(renderPass);
	mPipelines[key] = newVariant;

	return newVariant;
}

VkPipeline RHIGraphicsPipelineState::createVariant(RHIRenderPass * renderPass)
{
	// shader
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = mVertexProgram->getHandle();
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = mFragmentProgram->getHandle();
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
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

	// vertex input
	// notre:顶点数据布局暂时为固定形式,大版本稳定后将引进更加灵活的顶点输入布局
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, normal);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, uv);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	// 绘制方式
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = NULL;
	viewportState.flags = 0;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
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
	pipelineInfo.layout = mPipelineLayout;
	// render pass的取值不应该有pipeline判断,较好的方案为外部预先设置好renderpass
	pipelineInfo.renderPass = renderPass->getVkRenderPass(LoadMaskBits::LOAD_NONE, StoreMaskBits::STORE_NONE, ClearMaskBits::CLEAR_NONE);
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline pipeline;
	if (vkCreateGraphicsPipelines(mDevice->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}
	
	return pipeline;
}

