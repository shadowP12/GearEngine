#include "GpuBuffer.h"
#include "GearEngine.h"
#include "Utility/Log.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderCache.h"

#include <Blast/Gfx/GfxDefine.h>
#include <Blast/Gfx/GfxDevice.h>

namespace gear {
    GpuBuffer::GpuBuffer(blast::ResourceUsage usage, uint32_t size) {
        blast::GfxBufferDesc buffer_desc;
        buffer_desc.size = size;
        buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
        buffer_desc.res_usage = usage;
        buffer = gEngine.GetDevice()->CreateBuffer(buffer_desc);
    }

    GpuBuffer::~GpuBuffer() {
        gEngine.GetDevice()->DestroyBuffer(buffer);
    }

    uint32_t GpuBuffer::GetSize() {
        return buffer->desc.size;
    }

    void GpuBuffer::UpdateData(void* data, uint32_t size) {
        blast::GfxDevice* device = gEngine.GetDevice();
        blast::GfxCommandBuffer* cmd = device->RequestCommandBuffer(blast::QUEUE_COPY);
        device->UpdateBuffer(cmd, buffer, data, size);

        blast::GfxBufferBarrier barrier;
        barrier.buffer = buffer;
        barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(cmd, 1, &barrier, 0, nullptr);
    }

    void VertexBuffer::Builder::SetVertexCount(uint32_t count) {
        num_vertices = count;
    }

    void VertexBuffer::Builder::SetVertexLayoutType(VertexLayoutType type) {
        layout_type = type;
    }

//{ blast::SEMANTIC_POSITION, 0 },
//{ blast::SEMANTIC_NORMAL, 1 },
//{ blast::SEMANTIC_TANGENT, 2 },
//{ blast::SEMANTIC_BITANGENT, 3 },
//{ blast::SEMANTIC_COLOR, 4 },
//{ blast::SEMANTIC_TEXCOORD0, 5 },
//{ blast::SEMANTIC_TEXCOORD1, 6 },
//{ blast::SEMANTIC_JOINTS, 7 },
//{ blast::SEMANTIC_WEIGHTS, 8 },
//{ blast::SEMANTIC_CUSTOM0, 9 },
//{ blast::SEMANTIC_CUSTOM1, 10 },
//{ blast::SEMANTIC_CUSTOM2, 11 },
//{ blast::SEMANTIC_CUSTOM3, 12 },
//{ blast::SEMANTIC_CUSTOM4, 13 },
//{ blast::SEMANTIC_CUSTOM5, 14 }

    VertexBuffer* VertexBuffer::Builder::Build() {
        uint32_t stride = 0;
        blast::GfxInputLayout* vertex_layout = gEngine.GetRenderer()->GetVertexLayoutCache()->GetVertexLayout(layout_type);
        for (uint32_t i = 0; i < vertex_layout->elements.size(); ++i) {
            stride += vertex_layout->elements[i].size;
        }
        buffer_size = stride * num_vertices;

        return new VertexBuffer(this);
    }

    VertexBuffer::VertexBuffer(Builder* builder)
    : GpuBuffer(blast::RESOURCE_USAGE_VERTEX_BUFFER, builder->buffer_size) {
        layout_type = builder->layout_type;
    }

    VertexBuffer::~VertexBuffer() {
    }

    void IndexBuffer::Builder::SetIndexType(blast::IndexType type) {
        index_type = type;
    }

    void IndexBuffer::Builder::SetIndexCount(uint32_t count) {
        num_indices = count;
    }

    IndexBuffer* IndexBuffer::Builder::Build() {
        if (index_type == blast::INDEX_TYPE_UINT16) {
            buffer_size = sizeof(uint16_t) * num_indices;
        } else {
            buffer_size = sizeof(uint32_t) * num_indices;
        }
        return new IndexBuffer(this);
    }

    IndexBuffer::IndexBuffer(Builder* builder)
    : GpuBuffer(blast::RESOURCE_USAGE_INDEX_BUFFER, builder->buffer_size){
        index_type = builder->index_type;
    }

    IndexBuffer::~IndexBuffer() {
    }

    UniformBuffer::UniformBuffer(uint32_t size)
    : GpuBuffer(blast::RESOURCE_USAGE_UNIFORM_BUFFER, size) {
    }

    UniformBuffer::~UniformBuffer() {
    }
}