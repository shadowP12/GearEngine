#ifndef RHI_VERTEX_INPUT_MANAGER_H
#define RHI_VERTEX_INPUT_MANAGER_H
#include "RHI/RHIDefine.h"
#include "Utility/module.h"
#include "Mesh/VertexDescription.h"
#include <unordered_map>

class RHIVertexInputManager : public Module<RHIVertexInputManager>
{
public:
	RHIVertexInputManager();
	~RHIVertexInputManager();
	VkPipelineVertexInputStateCreateInfo getVertexInput(VertexDesc& vertexDesc);
private:
	VkPipelineVertexInputStateCreateInfo createVariant(VertexDesc& vertexDesc);
private:
	std::unordered_map<size_t, VkPipelineVertexInputStateCreateInfo> mVertexInputs;
};
#endif