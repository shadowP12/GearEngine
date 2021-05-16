#include "Scene/Components/CRenderable.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Renderer/Renderer.h"

namespace gear {
    CRenderable::CRenderable(Entity* entity)
            :Component(entity) {
    }

    CRenderable::~CRenderable() {
    }

    void CRenderable::setPrimitive(RenderPrimitive primitive) {
        mPrimitive = primitive;
    }

    void CRenderable::setVertexBuffer(VertexBuffer* buffer) {
        mVertexBuffer = buffer;
    }

    void CRenderable::setIndexBuffer(IndexBuffer* buffer) {
        mIndexBufferr = buffer;
    }

    void CRenderable::setMaterialInstance(MaterialInstance* instance) {
        mMaterialInstance = instance;
    }

    void CRenderable::updateRenderableBuffer() {
        // TODO: 添加RenderableBuffer
    }

}