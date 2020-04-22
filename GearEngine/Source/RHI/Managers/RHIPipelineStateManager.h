#ifndef RHI_PIPELINESTATEMANAGER_H
#define RHI_PIPELINESTATEMANAGER_H

#include <map>
#include <unordered_map>

class RHIDevice;
class RHIPipelineState;
class RHIProgram;
class RHIRenderPass;

class RHIPipelineStateManager
{
public:
    RHIPipelineStateManager(RHIDevice* device);
    ~RHIPipelineStateManager();
    RHIPipelineState* getPipelineState(RHIRenderPass* renderPass, RHIProgram* vertexProgram,
                                       RHIProgram* fragmentProgram);

private:
    struct VariantKey
    {
        VariantKey(uint32_t inRenderpass, uint32_t inVertexProgram, uint32_t inFragmentProgram);

        class HashFunction
        {
        public:
            std::size_t operator()(const VariantKey& key) const;
        };

        class EqualFunction
        {
        public:
            bool operator()(const VariantKey& lhs, const VariantKey& rhs) const;
        };

        uint32_t renderpass;
        uint32_t vertexProgram;
        uint32_t fragmentProgram;
    };
private:
    RHIDevice* mDevice;
    std::unordered_map<VariantKey, RHIPipelineState*, VariantKey::HashFunction, VariantKey::EqualFunction> mPipelineStates;
};


#endif
