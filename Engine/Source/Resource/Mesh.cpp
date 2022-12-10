#include "Mesh.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"

namespace gear {
    Mesh::~Mesh() {
        sub_meshs.clear();
    }

    std::vector<std::shared_ptr<Mesh::SubMesh>> &Mesh::GetSubMeshs() {
        return sub_meshs;
    }

    SubMeshData::Builder& SubMeshData::Builder::SetVertexCount(uint32_t vertex_count) {
        this->vertex_count = vertex_count;
        return *this;
    }

    SubMeshData::Builder& SubMeshData::Builder::SetVertexLayout(gear::VertexLayoutType vertex_layout) {
        this->vertex_layout = vertex_layout;
        return *this;
    }

    SubMeshData::Builder& SubMeshData::Builder::SetVertexData(uint8_t* vertex_data, uint32_t vertex_data_size) {
        this->vertex_data = new uint8_t[vertex_data_size];
        memcpy(this->vertex_data, vertex_data, vertex_data_size);
        this->vertex_data_size = vertex_data_size;
        return *this;
    }

    SubMeshData::Builder& SubMeshData::Builder::SetIndexCount(uint32_t index_count) {
        this->index_count = index_count;
        return *this;
    }

    SubMeshData::Builder& SubMeshData::Builder::SetIndexType(blast::IndexType index_type) {
        this->index_type = index_type;
        return *this;
    }

    SubMeshData::Builder& SubMeshData::Builder::SetIndexData(uint8_t* index_data, uint32_t index_data_size) {
        this->index_data = new uint8_t[index_data_size];
        memcpy(this->index_data, index_data, index_data_size);
        this->index_data_size = index_data_size;
        return *this;
    }

    std::shared_ptr<SubMeshData> SubMeshData::Builder::Build() {
        return std::shared_ptr<SubMeshData>(new SubMeshData(this));
    }

    SubMeshData::SubMeshData(gear::SubMeshData::Builder *builder) {
        vertex_count = builder->vertex_count;
        vertex_layout = builder->vertex_layout;
        vertex_data = builder->vertex_data;
        vertex_data_size = builder->vertex_data_size;
        index_count = builder->index_count;
        index_type = builder->index_type;
        index_data = builder->index_data;
        index_data_size = builder->index_data_size;
    }

    SubMeshData::~SubMeshData() {
        SAFE_DELETE_ARRAY(vertex_data);
        SAFE_DELETE_ARRAY(index_data);
    }

    MeshData::Builder& MeshData::Builder::AddSubMeshData(std::shared_ptr<SubMeshData> sub_mesh_data) {
        this->sub_mesh_datas.push_back(sub_mesh_data);
        return *this;
    }

    std::shared_ptr<MeshData> MeshData::Builder::Build() {
        return std::shared_ptr<MeshData>(new MeshData(this));
    }

    MeshData::MeshData(gear::MeshData::Builder* builder) {
        sub_mesh_datas = builder->sub_mesh_datas;
    }

    MeshData::~MeshData() {
    }

    std::shared_ptr<class Mesh> MeshData::LoadMesh() {
        Renderer* renderer = gEngine.GetRenderer();
        blast::GfxDevice* device = renderer->GetDevice();
        blast::GfxCommandBuffer* cmd = renderer->GetCurrentCommandBuffer();

        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
        for (auto sub_mesh_data : sub_mesh_datas) {
            std::shared_ptr<Mesh::SubMesh> sub_mesh = std::make_shared<Mesh::SubMesh>();

            sub_mesh->vertex_count = sub_mesh_data->vertex_count;
            sub_mesh->vertex_layout = sub_mesh_data->vertex_layout;

            blast::GfxBufferDesc buffer_desc{};
            buffer_desc.size = sub_mesh_data->vertex_data_size;
            buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
            buffer_desc.res_usage = blast::RESOURCE_USAGE_VERTEX_BUFFER;
            sub_mesh->vertex_buf = std::shared_ptr<blast::GfxBuffer>(device->CreateBuffer(buffer_desc));
            renderer->UpdateVertexBuffer(cmd, sub_mesh->vertex_buf.get(), sub_mesh_data->vertex_data, sub_mesh_data->vertex_data_size);

            sub_mesh->index_count = sub_mesh_data->index_count;
            sub_mesh->index_type = sub_mesh_data->index_type;

            buffer_desc.size = sub_mesh_data->index_data_size;
            buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
            buffer_desc.res_usage = blast::RESOURCE_USAGE_INDEX_BUFFER;
            sub_mesh->index_buf = std::shared_ptr<blast::GfxBuffer>(device->CreateBuffer(buffer_desc));
            renderer->UpdateIndexBuffer(cmd, sub_mesh->index_buf.get(), sub_mesh_data->index_data, sub_mesh_data->index_data_size);

            mesh->sub_meshs.push_back(sub_mesh);
        }
        return mesh;
    }
}