#ifndef RHI_PIPELINE_STATE_H
#define RHI_PIPELINE_STATE_H
#include "RHIDefine.h"
#include "RHIProgram.h"
#include "RHIProgramParam.h"
#include "RHIRenderPass.h"
#include "Utility/Hash.h"

#include <unordered_map>

class RHIDevice;
class RHIProgram;
class RHIRenderPass;

class RHIPipelineState
{
public:
    RHIPipelineState(RHIDevice* device, RHIRenderPass* renderPass, RHIProgram* vertexProgram,
                             RHIProgram* fragmentProgram);
	~RHIPipelineState();
	VkPipelineLayout getLayout() { return mPipelineLayout; }
private:
	RHIDevice* mDevice;
	VkPipeline mPipeline;
	VkPipelineLayout mPipelineLayout;
};

#endif