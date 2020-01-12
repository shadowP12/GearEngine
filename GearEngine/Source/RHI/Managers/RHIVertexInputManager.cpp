#include "RHIVertexInputManager.h"
#include <array>
#include <vector>
RHIVertexInputManager::RHIVertexInputManager()
{
}

RHIVertexInputManager::~RHIVertexInputManager()
{
}

VkPipelineVertexInputStateCreateInfo RHIVertexInputManager::getVertexInput(VertexDesc& vertexDesc)
{
	size_t hash = vertexDesc.getHash();
	auto iterFind = mVertexInputs.find(hash);
	if (iterFind != mVertexInputs.end())
		return iterFind->second;

	VkPipelineVertexInputStateCreateInfo newVariant = createVariant(vertexDesc);
	mVertexInputs[hash] = newVariant;

	return newVariant;
}

VkPipelineVertexInputStateCreateInfo RHIVertexInputManager::createVariant(VertexDesc& vertexDesc)
{
	// note:固定设置shader的绑定布局
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = vertexDesc.getVertexStride();
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	
	for (int i = 0; i < (int)vertexDesc.getNumElements(); i++)
	{
		VertexElement element = vertexDesc.getElement(i);
		VkVertexInputAttributeDescription attributeDesc;
		attributeDesc.binding = 0;
		attributeDesc.location = i;
		// todo:修改成自适应数据格式
		attributeDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDesc.offset = element.getOffset();
		attributeDescriptions.push_back(attributeDesc);
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	return vertexInputInfo;
}