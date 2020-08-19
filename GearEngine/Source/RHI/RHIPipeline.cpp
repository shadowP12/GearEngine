#include "RHIPipeline.h"
#include "RHIDevice.h"
#include "RHIProgram.h"
#include "RHIRenderPass.h"
#include "RHIDescriptorSet.h"
#include "Math/GMath.h"
#include <array>
#include <vector>

RHIGraphicsPipeline::RHIGraphicsPipeline(RHIDevice* device, const RHIGraphicsPipelineInfo& info)
	:mDevice(device)
{
    mVertexProgram = info.vertexProgram;
    mFragmentProgram = info.fragmentProgram;

    std::vector<VkDescriptorSetLayout> setLayouts;
    for (int i = 0; i < info.descriptorSetCount; ++i)
    {
        setLayouts.push_back(info.descriptorSets[i]->getLayout());
    }

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = setLayouts.size();
	pipelineLayoutInfo.pSetLayouts = setLayouts.data();

	CHECK_VKRESULT(vkCreatePipelineLayout(mDevice->getDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout));

    // shader
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = info.vertexProgram->getHandle();
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = info.fragmentProgram->getHandle();
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    //dynamic state
    VkDynamicState dynamicStateEnables[5];
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = NULL;
    dynamicState.pDynamicStates = dynamicStateEnables;
    dynamicState.dynamicStateCount = 0;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

    uint32_t inputBindingCount = 0;
    VkVertexInputBindingDescription inputBindings[8] = { { 0 } };
    uint32_t inputAttributeCount = 0;
    VkVertexInputAttributeDescription inputAttributes[8] = { { 0 } };

    uint32_t attribCount = info.vertexLayout.attribCount;
    uint32_t bindingValue = UINT32_MAX;

    for (uint32_t i = 0; i < attribCount; ++i)
    {
        const VertexAttrib* attrib = &(info.vertexLayout.attribs[i]);

        if (bindingValue != attrib->binding)
        {
            bindingValue = attrib->binding;
            ++inputBindingCount;
        }

        inputBindings[inputBindingCount - 1].binding = bindingValue;
        if (attrib->rate == VERTEX_ATTRIB_RATE_INSTANCE)
        {
            inputBindings[inputBindingCount - 1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        }
        else
        {
            inputBindings[inputBindingCount - 1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        }
        inputBindings[inputBindingCount - 1].stride += getBitSizeOfBlock(attrib->format) / 8;

        inputAttributes[inputAttributeCount].location = attrib->location;
        inputAttributes[inputAttributeCount].binding = attrib->binding;
        inputAttributes[inputAttributeCount].format = attrib->format;
        inputAttributes[inputAttributeCount].offset = attrib->offset;
        ++inputAttributeCount;
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = inputBindingCount;
    vertexInputInfo.pVertexBindingDescriptions = inputBindings;
    vertexInputInfo.vertexAttributeDescriptionCount = inputAttributeCount;
    vertexInputInfo.pVertexAttributeDescriptions = inputAttributes;

    // todo
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = info.topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = NULL;
    viewportState.flags = 0;
    viewportState.viewportCount = 1;
    viewportState.pViewports = NULL;
    viewportState.scissorCount = 1;
    viewportState.pScissors = NULL;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = info.rasterizerState.depthClampEnable;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = info.rasterizerState.polygonMode;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = info.rasterizerState.cullMode;
    rasterizer.frontFace = info.rasterizerState.frontFace;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState attachments[4] = { { 0 } };
    for (int i = 0; i < 4; ++i)
    {
        if (info.blendState.renderTargetMask & (1 << i))
        {
            VkBool32 blendEnable =
                    (info.blendState.srcFactors[i] != VK_BLEND_FACTOR_ONE ||
                    info.blendState.dstFactors[i] != VK_BLEND_FACTOR_ZERO ||
                    info.blendState.srcAlphaFactors[i] != VK_BLEND_FACTOR_ONE ||
                    info.blendState.dstAlphaFactors[i] != VK_BLEND_FACTOR_ZERO);

            attachments[i].blendEnable = blendEnable;
            attachments[i].srcColorBlendFactor = info.blendState.srcFactors[i];
            attachments[i].dstColorBlendFactor = info.blendState.dstFactors[i];
            attachments[i].colorBlendOp = info.blendState.blendModes[i];
            attachments[i].srcAlphaBlendFactor = info.blendState.srcAlphaFactors[i];
            attachments[i].dstAlphaBlendFactor = info.blendState.dstAlphaFactors[i];
            attachments[i].alphaBlendOp = info.blendState.blendAlphaModes[i];
            attachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        }
    }

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_CLEAR;
    colorBlending.attachmentCount = info.renderTargetCount;
    colorBlending.pAttachments = attachments;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = info.depthStencilState.depthTest;
    depthStencil.depthWriteEnable = info.depthStencilState.depthWrite;
    depthStencil.depthCompareOp = info.depthStencilState.depthFunc;
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
    pipelineInfo.renderPass = info.renderPass->getHandle();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    CHECK_VKRESULT(vkCreateGraphicsPipelines(mDevice->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline));
}

RHIGraphicsPipeline::~RHIGraphicsPipeline()
{
	vkDestroyPipelineLayout(mDevice->getDevice(), mPipelineLayout, nullptr);
    vkDestroyPipeline(mDevice->getDevice(), mPipeline, nullptr);
}

RHIComputePipeline::RHIComputePipeline(RHIDevice *device, const RHIComputePipelineInfo &info)
    :mDevice(device)
{
    mComputeProgram = info.computeProgram;
    std::vector<VkDescriptorSetLayout> setLayouts;
    for (int i = 0; i < info.descriptorSetCount; ++i)
    {
        setLayouts.push_back(info.descriptorSets[i]->getLayout());
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = setLayouts.size();
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();
    CHECK_VKRESULT(vkCreatePipelineLayout(mDevice->getDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout));

    // shader
    VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = info.computeProgram->getHandle();
    computeShaderStageInfo.pName = "main";

    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = computeShaderStageInfo;
    pipelineInfo.layout = mPipelineLayout;

    CHECK_VKRESULT(vkCreateComputePipelines(mDevice->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline));
}

RHIComputePipeline::~RHIComputePipeline()
{
    vkDestroyPipelineLayout(mDevice->getDevice(), mPipelineLayout, nullptr);
    vkDestroyPipeline(mDevice->getDevice(), mPipeline, nullptr);
}