#include "RHIPipelineStateManager.h"
#include "../RHIDevice.h"
#include "../RHIRenderPass.h"
#include "../RHIPipelineState.h"

RHIPipelineStateManager::VariantKey::VariantKey(uint32_t inRenderpass, uint32_t inVertexProgram, uint32_t inFragmentProgram)
{
    renderpass = inRenderpass;
    inVertexProgram = inVertexProgram;
    inFragmentProgram = inFragmentProgram;
}

size_t RHIPipelineStateManager::VariantKey::HashFunction::operator()(const VariantKey& v) const
{
    size_t hash = 0;
    Hash(hash, v.renderpass);
    Hash(hash, v.vertexProgram);
    Hash(hash, v.fragmentProgram);
    return hash;
}

bool RHIPipelineStateManager::VariantKey::EqualFunction::operator()(
        const VariantKey& lhs, const VariantKey& rhs) const
{
    if (lhs.renderpass != rhs.renderpass)
        return false;
    if (lhs.vertexProgram != rhs.vertexProgram)
        return false;
    if (lhs.fragmentProgram != rhs.fragmentProgram)
        return false;
    return true;
}

RHIPipelineStateManager::RHIPipelineStateManager(RHIDevice* device)
{

}

RHIPipelineStateManager::~RHIPipelineStateManager()
{

}

RHIPipelineState* RHIPipelineStateManager::getPipelineState(RHIRenderPass* renderPass, RHIProgram* vertexProgram,
                                   RHIProgram* fragmentProgram)
{
    VariantKey key(renderPass->getID(), vertexProgram->getID(), fragmentProgram->getID());
    auto iterFind = mPipelineStates.find(key);
    if (iterFind != mPipelineStates.end())
        return iterFind->second;

    RHIPipelineState* newVariant = new RHIPipelineState(mDevice, renderPass, vertexProgram, fragmentProgram);
    mPipelineStates[key] = newVariant;
    return newVariant;

}
