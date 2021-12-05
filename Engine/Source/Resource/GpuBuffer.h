#pragma once
#include "Renderer/RenderData.h"

#include <Blast/Gfx/GfxDefine.h>

#include <vector>
#include <unordered_map>

namespace gear {
    class GpuBuffer {
    public:
        GpuBuffer(blast::ResourceUsage usage, uint32_t size);

        virtual ~GpuBuffer();

        void UpdateData(void* data, uint32_t size);

        uint32_t GetSize();

        blast::GfxBuffer* GetHandle() { return buffer; }

    protected:
        blast::GfxBuffer* buffer = nullptr;
    };

    class VertexBuffer : public GpuBuffer {
    public:
        class Builder {
        public:
            Builder() = default;

            ~Builder() = default;

            void SetVertexCount(uint32_t count);

            void SetVertexLayoutType(VertexLayoutType type);

            VertexBuffer* Build();

        private:
            friend class VertexBuffer;
            uint32_t num_vertices;
            uint32_t buffer_size;
            VertexLayoutType layout_type;
        };

        ~VertexBuffer();

        VertexLayoutType GetVertexLayoutType() { return layout_type; }

    private:
        VertexBuffer(Builder* builder);

    private:
        VertexLayoutType layout_type;
    };

    class IndexBuffer : public GpuBuffer {
    public:
        class Builder {
        public:
            Builder() = default;

            ~Builder() = default;

            void SetIndexType(blast::IndexType type);

            void SetIndexCount(uint32_t count);

            IndexBuffer* Build();

        private:
            friend class IndexBuffer;
            uint32_t num_indices;
            uint32_t buffer_size;
            blast::IndexType index_type;
        };

        ~IndexBuffer();

        blast::IndexType GetIndexType() { return index_type; }

    private:
        IndexBuffer(Builder* builder);

    private:
        blast::IndexType index_type;
    };

    class UniformBuffer : public GpuBuffer {
    public:
        UniformBuffer(uint32_t size);

        ~UniformBuffer();
    };
}