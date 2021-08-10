#include "GpuBuffer.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include "Renderer/CopyEngine.h"
#include "Utility/Log.h"
#include <blast/Gfx/GfxCommandBuffer.h>
#include <unordered_map>
namespace gear {
    GpuBuffer::GpuBuffer(blast::ResourceType type, uint32_t size) {
        blast::GfxContext* context = gEngine.GetRenderer()->GetContext();
        blast::GfxBufferDesc buffer_desc;
        buffer_desc.size = size;
        buffer_desc.type = type;
        buffer_desc.usage = blast::RESOURCE_USAGE_GPU_ONLY;
        _buffer = context->CreateBuffer(buffer_desc);
    }

    GpuBuffer::~GpuBuffer() {
        Renderer* renderer = gEngine.GetRenderer();
        renderer->Destroy(_buffer);
    }

    uint32_t GpuBuffer::GetSize() {
        return _buffer->GetSize();
    }

    void GpuBuffer::Update(void* data, uint32_t offset, uint32_t size) {
        Renderer* renderer = gEngine.GetRenderer();
        renderer->ExecRenderTask([this, renderer, offset, size, data](blast::GfxCommandBuffer* cmd) {
            blast::GfxBuffer* staging_buffer = renderer->AllocStageBuffer(size);
            staging_buffer->WriteData(offset, size, data);
            renderer->UseResource(staging_buffer);

            blast::GfxCopyToBufferRange range;
            range.size = size;
            range.src_offset = 0;
            range.dst_offset = 0;
            cmd->CopyToBuffer(staging_buffer, _buffer, range);

            blast::GfxBufferBarrier barrier;
            barrier.buffer = _buffer;
            barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
            cmd->SetBarrier(1, &barrier, 0, nullptr);
        });
    }

    void VertexBuffer::Builder::SetVertexCount(uint32_t count) {
        _num_vertices = count;
    }

    void VertexBuffer::Builder::SetAttribute(blast::ShaderSemantic semantic, blast::Format format) {
        blast::GfxVertexAttribute attribute;
        attribute.semantic = semantic;
        attribute.format = format;
        _attributes.push_back(attribute);
    }

    VertexBuffer* VertexBuffer::Builder::Build() {
        static const std::unordered_map<blast::ShaderSemantic, uint32_t> toLocation {
                { blast::SEMANTIC_POSITION, 0 },
                { blast::SEMANTIC_NORMAL, 1 },
                { blast::SEMANTIC_TANGENT, 2 },
                { blast::SEMANTIC_BITANGENT, 3 },
                { blast::SEMANTIC_COLOR, 4 },
                { blast::SEMANTIC_TEXCOORD0, 5 },
                { blast::SEMANTIC_TEXCOORD1, 6 },
                { blast::SEMANTIC_JOINTS, 7 },
                { blast::SEMANTIC_WEIGHTS, 8 },
                { blast::SEMANTIC_CUSTOM0, 9 },
                { blast::SEMANTIC_CUSTOM1, 10 },
                { blast::SEMANTIC_CUSTOM2, 11 },
                { blast::SEMANTIC_CUSTOM3, 12 },
                { blast::SEMANTIC_CUSTOM4, 13 },
                { blast::SEMANTIC_CUSTOM5, 14 }
        };
        Renderer* renderer = gEngine.GetRenderer();

        uint32_t offset = 0;
        for (int i = 0; i < _attributes.size(); ++i) {
            _attributes[i].location = toLocation.at(_attributes[i].semantic);
            _attributes[i].offset = offset;
            _attributes[i].size = blast::GetFormatStride(_attributes[i].format);
            uint32_t stride = _attributes[i].size;
            offset += stride;
        }
        _buffer_Size = offset * _num_vertices;

        return new VertexBuffer(this);
    }

    VertexBuffer::VertexBuffer(Builder* builder)
    : GpuBuffer(blast::RESOURCE_TYPE_VERTEX_BUFFER, builder->_buffer_Size) {
        _layout.num_attributes = builder->_attributes.size();
        for (int i = 0; i < builder->_attributes.size(); ++i) {
            _layout.attributes[i] = builder->_attributes[i];
        }
    }

    VertexBuffer::~VertexBuffer() {
    }

    void IndexBuffer::Builder::SetIndexType(blast::IndexType type) {
        _index_type = type;
    }

    void IndexBuffer::Builder::SetIndexCount(uint32_t count) {
        _num_indices = count;
    }

    IndexBuffer* IndexBuffer::Builder::Build() {
        if (_index_type == blast::INDEX_TYPE_UINT16) {
            _buffer_size = sizeof(uint16_t) * _num_indices;
        } else {
            _buffer_size = sizeof(uint32_t) * _num_indices;
        }
        return new IndexBuffer(this);
    }

    IndexBuffer::IndexBuffer(Builder* builder)
    : GpuBuffer(blast::ResourceType::RESOURCE_TYPE_INDEX_BUFFER, builder->_buffer_size){
        _index_type = builder->_index_type;
    }

    IndexBuffer::~IndexBuffer() {

    }

    UniformBuffer::UniformBuffer(uint32_t size)
    : GpuBuffer(blast::RESOURCE_TYPE_UNIFORM_BUFFER, size) {
    }

    UniformBuffer::~UniformBuffer() noexcept {

    }
}