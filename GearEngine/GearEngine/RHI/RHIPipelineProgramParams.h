#ifndef RHI_PIPELINR_PROGRAM_PARAMS
#define RHI_PIPELINE_PROGRAM_PARAMS
#include "RHIDefine.h"
#include "RHIProgramParam.h"

// note:目前只支持两种类型的着色器
struct RHIPipelineParamsInfo
{
	RHIParamInfo vertexParams;
	RHIParamInfo fragmentParams;
};


class RHIPipelineParams
{
public:
	RHIPipelineParams();
	~RHIPipelineParams();

private:

};

#endif