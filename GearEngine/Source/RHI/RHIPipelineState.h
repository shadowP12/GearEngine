#ifndef RHI_PIPELINE_STATE_H
#define RHI_PIPELINE_STATE_H
#include "RHIDefine.h"
#include "RHIProgram.h"
#include "RHIProgramParam.h"
#include "RHIRenderPass.h"
#include "Utility/Hash.h"

#include <unordered_map>

// note:暂不加入计算管线
// note:使用智能指针来传参会好一点?

struct RHIPipelineStateInfo
{
	//programs
	RHIProgram* vertexProgram = nullptr;
	RHIProgram* fragmentProgram = nullptr;

	//todo:render states
};

class RHIDevice;

class RHIGraphicsPipelineState
{
public:
	RHIGraphicsPipelineState(RHIDevice* device, const RHIPipelineStateInfo& info);
	~RHIGraphicsPipelineState();
	VkPipeline getPipeline(RHIRenderPass* renderPass);
	VkPipelineLayout getLayout() { return mPipelineLayout; }
	std::vector<VkDescriptorSet>& getDescSets() { return mDescriptorSets2; }
private:
	struct VariantKey
	{
		// note:开发阶段仅使用render pass生成hash key,后续还有绘制方式以及顶点输入布局
		VariantKey(uint32_t inRenderpassID);

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

		uint32_t renderpassID;
	};

	VkPipeline createVariant(RHIRenderPass* renderPass);

private:
	RHIDevice* mDevice;
	RHIProgram* mVertexProgram = nullptr;
	RHIProgram* mFragmentProgram = nullptr;
	RHIProgramParamList* mParamList = nullptr;
	std::unordered_map<VariantKey, VkPipeline, VariantKey::HashFunction, VariantKey::EqualFunction> mPipelines;
	// 构建一个巨大的DescriptorPool好处在于方便,但反过来会造成内存浪费
	// 现在方案为为每个RHIPipeline都拥有一个DescriptorPool
	VkDescriptorPool mDescriptorPool;
	std::map<uint32_t, VkDescriptorSetLayout> mDescriptorSetLayouts;
	std::map<uint32_t, VkDescriptorSet> mDescriptorSets;
	// 临时变量
	std::vector<VkDescriptorSet> mDescriptorSets2;
	VkPipelineLayout mPipelineLayout;
	std::vector<uint32_t> mSets;
};

#endif