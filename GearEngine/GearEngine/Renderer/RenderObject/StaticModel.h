#ifndef STATIC_MODEL_H
#define STATIC_MODEL_H
#include "../../Mesh/Mesh.h"
#include "../../Material/Material.h"
#include "../../SceneGraph/Node.h"
#include "../../RenderAPI/CommandBuffer.h"

struct UboScene
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class StaticModel
{
public:
	StaticModel();
	~StaticModel();
	void setMesh(std::shared_ptr<Mesh> mesh)
	{
		mMesh = mesh;
	}
	void setMaterial(std::shared_ptr<Material> mat)
	{
		mMat = mat;
	}
	void setNode(std::shared_ptr<Node> node)
	{
		mNode = node;
	}
	void setSceneData(glm::mat4 view, glm::mat4 proj)
	{
		UboScene us;
		us.model = mNode->getWorldMatrix();
		us.proj = proj;
		us.view = view;
		mMat->setUniform("scene",&us);
	}
	void render(CommandBuffer& commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mMat->getPipeline());

		VkBuffer vertexBuffers[] = { mMesh->getVertexBuffer()->getBuffer()->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer.getCommandBuffer(), 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer.getCommandBuffer(), mMesh->getIndexBuffer()->getBuffer()->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

		VkDescriptorSet set = mMat->getDescriptorSet();
		vkCmdBindDescriptorSets(commandBuffer.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mMat->getPipelineLayout(), 0, 1, &set, 0, nullptr);

		vkCmdDrawIndexed(commandBuffer.getCommandBuffer(), mMesh->getIndexBuffer()->getIndexCount(), 1, 0, 0, 0);
	}
private:
	uint32_t mId;
	std::shared_ptr<Mesh> mMesh;
	std::shared_ptr<Material> mMat;
	std::shared_ptr<Node> mNode;
};

StaticModel::StaticModel()
{
}

StaticModel::~StaticModel()
{
}

#endif