#include "RenderBuiltinResource.h"
#include "Renderer.h"

#include <Blast/Gfx/GfxContext.h>
#include <Blast/Gfx/GfxBuffer.h>

namespace gear {
    RenderBuiltinResource::RenderBuiltinResource(Renderer* renderer) {
        mRenderer = renderer;
        createQuadBuffer();
    }

    RenderBuiltinResource::~RenderBuiltinResource() {
        SAFE_DELETE(mQuadVertexBuffer);
        SAFE_DELETE(mQuadIndexBuffer);
    }

    void RenderBuiltinResource::createQuadBuffer() {
        float vertices[] = {
                -1.0f, -1.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
                -1.0f, 1.0f, 0.0f, 1.0f
        };

        uint32_t indices[] = {
                0, 1, 2, 2, 3, 0
        };

        Blast::GfxContext* context = mRenderer->getContext();
        Blast::GfxBufferDesc bufferDesc;
        bufferDesc.size = sizeof(vertices);
        bufferDesc.type = Blast::RESOURCE_TYPE_VERTEX_BUFFER;
        bufferDesc.usage = Blast::RESOURCE_USAGE_CPU_TO_GPU;
        mQuadVertexBuffer = context->createBuffer(bufferDesc);
        mQuadVertexBuffer->writeData(0, sizeof(vertices), vertices);

        bufferDesc.size = sizeof(indices);
        bufferDesc.type = Blast::RESOURCE_TYPE_INDEX_BUFFER;
        bufferDesc.usage = Blast::RESOURCE_USAGE_CPU_TO_GPU;
        mQuadIndexBuffer = context->createBuffer(bufferDesc);
        mQuadIndexBuffer->writeData(0, sizeof(indices), indices);
    }
}