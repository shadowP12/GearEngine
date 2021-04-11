#pragma once

#include "Core/GearDefine.h"
#include "Resource/Resource.h"
#include <Blast/Gfx/GfxDefine.h>

namespace Blast {
    class GfxBuffer;
}

namespace gear {
    class MeshLayout {
    public:
        MeshLayout();
        void addAttribute(Blast::Format format, Blast::ShaderSemantic semantic);
        uint32_t getAttributeSize(Blast::ShaderSemantic semantic);
        uint32_t getAttributeOffset(Blast::ShaderSemantic semantic);
        uint32_t getAttributeStride();
    private:
        friend class Mesh;
        struct MeshAttribute {
            Blast::Format format;
            Blast::ShaderSemantic semantic;
        };
        std::vector<MeshAttribute> mAttributes;
        Blast::ShaderSemantic mLayout;
    };

    struct MeshDesc {
        uint32_t numVertices;
        uint32_t numIndices;
        MeshLayout layout;
    };

    class Mesh : public Resource {
    public:
        Mesh(const MeshDesc& desc);
        ~Mesh();
        void setIndexes(void* data, uint32_t size);
        void setAttribute(Blast::ShaderSemantic semantic, void* data, uint32_t size);
        uint8_t* getIndices();
        uint8_t* getVertices();
        uint32_t getNumVertices() { return mNumVertices; }
        uint32_t getNumIndices() { return mNumIndices; }
        uint32_t getVertexBufferSize();
        uint32_t getIndexBufferSize();
        void updateRenderData();
    private:
        ResourceType mType = ResourceType::MESH;
        MeshLayout mLayout;
        uint8_t* mData;
        uint32_t mNumVertices;
        uint32_t mNumIndices;
        Blast::GfxBuffer* mVertexBuffer;
        Blast::GfxBuffer* mIndexBuffer;
    };
}