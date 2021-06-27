#include "GpuBuffer.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include "Renderer/CopyEngine.h"
#include "Utility/Log.h"
#include <Blast/Gfx/GfxCommandBuffer.h>
#include <unordered_map>
namespace gear {
    GpuBuffer::GpuBuffer(Blast::ResourceType type, uint32_t size) {
        mSize = size;
        Blast::GfxContext* context = gEngine.getRenderer()->getContext();
        Blast::GfxBufferDesc bufferDesc;
        bufferDesc.size = size;
        bufferDesc.type = type;
        bufferDesc.usage = Blast::RESOURCE_USAGE_GPU_ONLY;
        mBuffer = context->createBuffer(bufferDesc);
    }

    GpuBuffer::~GpuBuffer() {
        Renderer* renderer = gEngine.getRenderer();
        CopyEngine* copyEngine = renderer->getCopyEngine();
        copyEngine->destroy(mBuffer);
    }

    uint32_t GpuBuffer::getSize() {
        return mSize;
    }

    void GpuBuffer::update(void* data, uint32_t offset, uint32_t size) {
        // TODO: StagingBuffer部分需要优化
        Renderer* renderer = gEngine.getRenderer();
        CopyEngine* copyEngine = renderer->getCopyEngine();
        Blast::GfxContext* context = renderer->getContext();
        Blast::GfxQueue* queue = renderer->getQueue();

        Blast::GfxBufferDesc bufferDesc;
        bufferDesc.size = size;
        bufferDesc.type = Blast::RESOURCE_TYPE_RW_BUFFER;
        bufferDesc.usage = Blast::RESOURCE_USAGE_CPU_TO_GPU;
        Blast::GfxBuffer* stagingBuffer = context->createBuffer(bufferDesc);
        stagingBuffer->writeData(offset, size, data);

        CopyCommand* copyCmd = copyEngine->getActiveCommand();
        copyCmd->cmd->begin();
        copyCmd->cmd->copyToBuffer(stagingBuffer, 0, mBuffer, 0, size);
        Blast::GfxBufferBarrier barrier;
        barrier.buffer = mBuffer;
        barrier.newState = Blast::RESOURCE_STATE_SHADER_RESOURCE;
        copyCmd->cmd->setBarrier(1, &barrier, 0, nullptr);
        copyCmd->cmd->end();

        copyEngine->acquireStage(1);
        copyCmd->callbacks.push_back([stagingBuffer, copyEngine]() {
            copyEngine->releaseStage(nullptr);
            delete stagingBuffer;
        });

        Blast::GfxSubmitInfo submitInfo;
        submitInfo.cmdBufCount = 1;
        submitInfo.cmdBufs = &copyCmd->cmd;
        submitInfo.signalFence = copyCmd->fence;
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.waitSemaphores = nullptr;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.signalSemaphores = nullptr;
        queue->submit(submitInfo);
    }

    void VertexBuffer::Builder::vertexCount(uint32_t count) {
        mVertexCount = count;
    }

    void VertexBuffer::Builder::attribute(Blast::ShaderSemantic semantic, Blast::Format format) {
        Blast::GfxVertexAttrib attrib;
        attrib.semantic = semantic;
        attrib.format = format;
        mAttributes.push_back(attrib);
    }

    VertexBuffer* VertexBuffer::Builder::build() {
        static const std::unordered_map<Blast::ShaderSemantic, uint32_t> toLocation {
                { Blast::SEMANTIC_POSITION, 0 },
                { Blast::SEMANTIC_NORMAL, 1 },
                { Blast::SEMANTIC_TANGENT, 2 },
                { Blast::SEMANTIC_BITANGENT, 3 },
                { Blast::SEMANTIC_COLOR, 4 },
                { Blast::SEMANTIC_TEXCOORD0, 5 },
                { Blast::SEMANTIC_TEXCOORD1, 6 },
                { Blast::SEMANTIC_JOINTS, 7 },
                { Blast::SEMANTIC_WEIGHTS, 8 },
                { Blast::SEMANTIC_CUSTOM0, 9 },
                { Blast::SEMANTIC_CUSTOM1, 10 },
                { Blast::SEMANTIC_CUSTOM2, 11 },
                { Blast::SEMANTIC_CUSTOM3, 12 },
                { Blast::SEMANTIC_CUSTOM4, 13 },
                { Blast::SEMANTIC_CUSTOM5, 14 }
        };
        Renderer* renderer = gEngine.getRenderer();
        Blast::GfxContext* context = renderer->getContext();

        uint32_t offset = 0;
        for (int i = 0; i < mAttributes.size(); ++i) {
            mAttributes[i].location = toLocation.at(mAttributes[i].semantic);
            mAttributes[i].offset = offset;
            mAttributes[i].size = context->getFormatStride(mAttributes[i].format);
            uint32_t stride = mAttributes[i].size;
            offset += stride;
        }
        mBufferSize = offset * mVertexCount;

        return new VertexBuffer(this);
    }

    VertexBuffer::VertexBuffer(Builder* builder)
    : GpuBuffer(Blast::RESOURCE_TYPE_VERTEX_BUFFER, builder->mBufferSize) {
        mLayout.attribCount = builder->mAttributes.size();
        for (int i = 0; i < builder->mAttributes.size(); ++i) {
            mLayout.attribs[i] = builder->mAttributes[i];
        }
    }

    VertexBuffer::~VertexBuffer() {
    }

    void IndexBuffer::Builder::indexType(Blast::IndexType type) {
        mIndexType = type;
    }

    void IndexBuffer::Builder::indexCount(uint32_t count) {
        mIndexCount = count;
    }

    IndexBuffer* IndexBuffer::Builder::build() {
        if (mIndexType == Blast::INDEX_TYPE_UINT16) {
            mBufferSize = sizeof(uint16_t) * mIndexCount;
        } else {
            mBufferSize = sizeof(uint32_t) * mIndexCount;
        }
        return new IndexBuffer(this);
    }

    IndexBuffer::IndexBuffer(Builder* builder)
    : GpuBuffer(Blast::ResourceType::RESOURCE_TYPE_INDEX_BUFFER, builder->mBufferSize){
        mIndexType = builder->mIndexType;
    }

    IndexBuffer::~IndexBuffer() {

    }

    UniformBuffer::UniformBuffer(uint32_t size)
    : GpuBuffer(Blast::RESOURCE_TYPE_UNIFORM_BUFFER, size) {
    }

    UniformBuffer::~UniformBuffer() noexcept {

    }
}