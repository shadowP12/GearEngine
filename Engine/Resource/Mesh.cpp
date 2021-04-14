#include "Mesh.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include "Renderer/CopyEngine.h"
#include <Blast/Gfx/GfxContext.h>
#include <Blast/Gfx/GfxBuffer.h>

namespace gear {

    MeshLayout::MeshLayout() {
    }

    void MeshLayout::addAttribute(Blast::Format format, Blast::ShaderSemantic semantic) {
        MeshAttribute newAttribute;
        newAttribute.format = format;
        newAttribute.semantic = semantic;
        mAttributes.push_back(newAttribute);
    }

    uint32_t MeshLayout::getAttributeOffset(Blast::ShaderSemantic semantic) {
        uint32_t offset = 0;
        for (auto& attribute : mAttributes) {
            if (attribute.semantic == semantic)
                break;
            Blast::GfxContext* context = gEngine.getRenderer()->getContext();
            offset += context->getFormatStride(attribute.format);
        }

        return offset;
    }

    uint32_t MeshLayout::getAttributeSize(Blast::ShaderSemantic semantic) {
        for (auto& attribute : mAttributes) {
            if (attribute.semantic == semantic) {
                Blast::GfxContext* context = gEngine.getRenderer()->getContext();
                return context->getFormatStride(attribute.format);
            }
        }

        return 0;
    }

    uint32_t MeshLayout::getAttributeStride() {
        uint32_t stride = 0;
        for (uint32_t i = 0; i < mAttributes.size(); i++) {
            Blast::GfxContext* context = gEngine.getRenderer()->getContext();
            stride += context->getFormatStride(mAttributes[i].format);
        }

        return stride;
    }

    Mesh::Mesh(const MeshDesc& desc) {
        mLayout = desc.layout;
        mNumVertices = desc.numVertices;
        mNumIndices = desc.numIndices;
        uint32_t totalSize = getVertexBufferSize() + getIndexBufferSize();
        mData = new uint8_t[totalSize];

        Blast::GfxContext* context = gEngine.getRenderer()->getContext();
        Blast::GfxBufferDesc bufferDesc;
        bufferDesc.size = getVertexBufferSize();
        bufferDesc.type = Blast::RESOURCE_TYPE_VERTEX_BUFFER;
        bufferDesc.usage = Blast::RESOURCE_USAGE_CPU_TO_GPU;
        mVertexBuffer = context->createBuffer(bufferDesc);

        bufferDesc.size = getIndexBufferSize();
        bufferDesc.type = Blast::RESOURCE_TYPE_INDEX_BUFFER;
        bufferDesc.usage = Blast::RESOURCE_USAGE_CPU_TO_GPU;
        mIndexBuffer = context->createBuffer(bufferDesc);
    }

    Mesh::~Mesh() {
        SAFE_DELETE_ARRAY(mData);
        SAFE_DELETE(mVertexBuffer);
        SAFE_DELETE(mIndexBuffer);
    }

    uint32_t Mesh::getVertexBufferSize() {
        return mNumVertices * mLayout.getAttributeStride();
    }

    uint32_t Mesh::getIndexBufferSize() {
        return mNumIndices * sizeof(uint32_t);
    }

    void Mesh::setIndexes(void* data, uint32_t size) {
        uint8_t* dst = mData;
        uint8_t* src = (uint8_t*)data;

        memcpy(dst, src, size);
    }

    void Mesh::setAttribute(Blast::ShaderSemantic semantic, void* data, uint32_t size) {
        uint32_t attributeSize = mLayout.getAttributeSize(semantic);
        uint32_t totalSize = attributeSize * mNumVertices;

        if (totalSize != size)
            return;

        uint32_t offset = mLayout.getAttributeOffset(semantic);
        uint32_t stride = mLayout.getAttributeStride();

        uint8_t* dst = mData + mNumIndices * sizeof(uint32_t) + offset;
        uint8_t* src = (uint8_t*)data;
        for (uint32_t i = 0; i < mNumVertices; i++) {
            memcpy(dst, src, attributeSize);
            dst += stride;
            src += attributeSize;
        }
    }

    uint8_t* Mesh::getIndices() {
        return mData;
    }

    uint8_t* Mesh::getVertices() {
        return mData + getIndexBufferSize();
    }

    void Mesh::updateRenderData() {
        mVertexBuffer->writeData(0, getVertexBufferSize(), getVertices());
        mIndexBuffer->writeData(0, getIndexBufferSize(), getIndices());
    }
}
