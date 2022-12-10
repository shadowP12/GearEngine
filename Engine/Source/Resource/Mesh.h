#pragma once
#include "Core/GearDefine.h"
#include "Renderer/RenderData.h"

#include <GfxDefine.h>
#include <vector>
#include <memory>

namespace gear {

    class MeshData;
    class SubMeshData;

    class Mesh {
    public:
        struct SubMesh {
            uint32_t vertex_count = 0;
            VertexLayoutType vertex_layout;
            uint32_t index_count = 0;
            blast::IndexType index_type;
            std::shared_ptr<blast::GfxBuffer> vertex_buf;
            std::shared_ptr<blast::GfxBuffer> index_buf;
        };

        Mesh() = default;

        ~Mesh();

        std::vector<std::shared_ptr<SubMesh>>& GetSubMeshs();

    private:
        friend MeshData;
        std::vector<std::shared_ptr<SubMesh>> sub_meshs;
    };

    class SubMeshData {
    public:
        class Builder {
        public:
            Builder() = default;

            ~Builder() = default;

            Builder& SetVertexCount(uint32_t vertex_count);

            Builder& SetVertexLayout(VertexLayoutType vertex_layout);

            Builder& SetVertexData(uint8_t* vertex_data, uint32_t vertex_data_size);

            Builder& SetIndexCount(uint32_t index_count);

            Builder& SetIndexType(blast::IndexType index_type);

            Builder& SetIndexData(uint8_t* index_data, uint32_t index_data_size);

            std::shared_ptr<SubMeshData> Build();

        private:
            friend class SubMeshData;
            uint32_t vertex_count = 0;
            VertexLayoutType vertex_layout;
            uint8_t* vertex_data = nullptr;
            uint32_t vertex_data_size = 0;
            uint32_t index_count = 0;
            blast::IndexType index_type;
            uint8_t* index_data = nullptr;
            uint32_t index_data_size = 0;
        };

        ~SubMeshData();

    private:
        friend class MeshData;
        SubMeshData(Builder* builder);

    private:
        uint32_t vertex_count = 0;
        VertexLayoutType vertex_layout;
        uint8_t* vertex_data = nullptr;
        uint32_t vertex_data_size = 0;
        uint32_t index_count = 0;
        blast::IndexType index_type;
        uint8_t* index_data = nullptr;
        uint32_t index_data_size = 0;
    };

    class MeshData {
    public:
        class Builder {
        public:
            Builder() = default;

            ~Builder() = default;

            Builder& AddSubMeshData(std::shared_ptr<SubMeshData>);

            std::shared_ptr<MeshData> Build();

        private:
            friend class MeshData;
            std::vector<std::shared_ptr<SubMeshData>> sub_mesh_datas;
        };

        ~MeshData();

        std::shared_ptr<class Mesh> LoadMesh();

    private:
        MeshData(Builder* builder);

    private:
        std::vector<std::shared_ptr<SubMeshData>> sub_mesh_datas;
    };
}