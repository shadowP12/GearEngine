#include "RHIPipelineState.h"

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

}

RHIGraphicsPipelineState::~RHIGraphicsPipelineState()
{
}

VkPipeline RHIGraphicsPipelineState::getPipeline()
{
	return VkPipeline();
}
