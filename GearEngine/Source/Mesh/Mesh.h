#ifndef MESH_H
#define MESH_H
#include "../RenderAPI/VertexBuffer.h"
#include "../RenderAPI/IndexBuffer.h"
#include "../Resource/Resource.h"
#include "../Math/GMath.h"

struct Vertex 
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
};

struct SubMesh 
{
	uint32_t indexOffset;
	uint32_t indexCount;
};
class MeshImporter;
class Mesh : public Resource
{
public:
	Mesh() {}
	~Mesh()
	{}

	void setVertexBuffer(std::shared_ptr<VertexBuffer> vb)
	{
		mVertexBuffer = vb;
	}
	std::shared_ptr<VertexBuffer> getVertexBuffer() { return mVertexBuffer; }

	void setIndexBuffer(std::shared_ptr<IndexBuffer> ib)
	{
		mIndexBuffer = ib;
	}
	std::shared_ptr<IndexBuffer> getIndexBuffer() { return mIndexBuffer; }

	void setSubMesh(std::vector<SubMesh>& sub)
	{
		mSubMesh = sub;
	}
private:
	friend MeshImporter;
	std::vector<SubMesh> mSubMesh;
	std::shared_ptr<VertexBuffer> mVertexBuffer;
	std::shared_ptr<IndexBuffer> mIndexBuffer;

};
#endif