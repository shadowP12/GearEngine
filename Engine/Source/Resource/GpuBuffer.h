#pragma once
#include <Blast/Gfx/GfxDefine.h>
#include <Blast/Gfx/GfxBuffer.h>
#include <Blast/Gfx/GfxContext.h>
#include <Blast/Gfx/GfxPipeline.h>
#include <vector>
namespace gear {
    class GpuBuffer {
    public:
        GpuBuffer(blast::ResourceType type, uint32_t size);

        virtual ~GpuBuffer();

        void Update(void* data, uint32_t offset, uint32_t size);

        uint32_t GetSize();

        blast::GfxBuffer* GetBuffer() { return _buffer; }

    protected:
        blast::GfxBuffer* _buffer = nullptr;
    };

    class VertexBuffer : public GpuBuffer {
    public:
        class Builder {
        public:
            Builder() = default;

            ~Builder() = default;

            void SetVertexCount(uint32_t count);

            void SetAttribute(blast::ShaderSemantic semantic, blast::Format format);

            VertexBuffer* Build();

        private:
            friend class VertexBuffer;
            uint32_t _num_vertices;
            uint32_t _buffer_Size;
            std::vector<blast::GfxVertexAttribute> _attributes;
        };

        ~VertexBuffer();

        blast::GfxVertexLayout GetVertexLayout() { return _layout; }

    private:
        VertexBuffer(Builder* builder);

    private:
        blast::GfxVertexLayout _layout;
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
            uint32_t _num_indices;
            uint32_t _buffer_size;
            blast::IndexType _index_type;
        };

        ~IndexBuffer();

        blast::IndexType GetIndexType() { return _index_type; }

    private:
        IndexBuffer(Builder* builder);

    private:
        blast::IndexType _index_type;
    };

    class UniformBuffer : public GpuBuffer {
    public:
        UniformBuffer(uint32_t size);

        ~UniformBuffer();
    };
}