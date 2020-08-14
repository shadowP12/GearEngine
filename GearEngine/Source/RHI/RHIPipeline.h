#ifndef RHI_PIPELINE_STATE_H
#define RHI_PIPELINE_STATE_H
#include "RHIDefine.h"
#include "RHIRenderState.h"

class RHIDevice;
class RHIProgram;
class RHIRenderPass;
class RHIDescriptorSet;

struct RHIGraphicsPipelineInfo
{
    RHIRenderPass* renderPass;
    RHIProgram* vertexProgram;
    RHIProgram* fragmentProgram;
    VertexLayout vertexLayout;
    BlendState blendState;
    DepthStencilState depthStencilState;
    RasterizerState rasterizerState;
    RHIDescriptorSet** descriptorSets;
    uint32_t descriptorSetCount;
    uint32_t renderTargetCount;
};

class RHIGraphicsPipeline
{
public:
    RHIGraphicsPipeline(RHIDevice* device, const RHIGraphicsPipelineInfo& info);
	~RHIGraphicsPipeline();
	VkPipelineLayout getLayout() { return mPipelineLayout; }
	VkPipeline getHandle() { return mPipeline; }
	RHIProgram* getVertexProgram() { return mVertexProgram; }
    RHIProgram* getFragmentProgram() { return mFragmentProgram; }
private:
	RHIDevice* mDevice;
	VkPipeline mPipeline;
	VkPipelineLayout mPipelineLayout;
    RHIProgram* mVertexProgram;
    RHIProgram* mFragmentProgram;
};

struct RHIComputePipelineInfo
{
    RHIProgram* computeProgram;
    RHIDescriptorSet** descriptorSets;
    uint32_t descriptorSetCount;
};

class RHIComputePipeline
{
public:
    RHIComputePipeline(RHIDevice* device, const RHIComputePipelineInfo& info);
    ~RHIComputePipeline();
    VkPipelineLayout getLayout() { return mPipelineLayout; }
    VkPipeline getHandle() { return mPipeline; }
    RHIProgram* getComputeProgram() { return mComputeProgram; }
private:
    RHIDevice* mDevice;
    VkPipeline mPipeline;
    VkPipelineLayout mPipelineLayout;
    RHIProgram* mComputeProgram;
};

#endif