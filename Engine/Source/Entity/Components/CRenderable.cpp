#include "Scene/Components/CRenderable.h"
#include "Scene/Components/CTransform.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderScene.h"
#include "Resource/GpuBuffer.h"

namespace gear {
    CRenderable::CRenderable(Entity* entity)
            :Component(entity) {
        mRenderableBuffer = new UniformBuffer(sizeof(ObjectUniforms));
    }

    CRenderable::~CRenderable() {
        SAFE_DELETE(mRenderableBuffer);
    }

    void CRenderable::resetPrimitives() {
        mPrimitives.clear();
    }

    void CRenderable::addPrimitive(const RenderPrimitive& primitive) {
        mPrimitives.push_back(primitive);
    }

    void CRenderable::setVertexBuffer(VertexBuffer* buffer) {
        mVertexBuffer = buffer;
    }

    void CRenderable::setIndexBuffer(IndexBuffer* buffer) {
        mIndexBufferr = buffer;
    }

    void CRenderable::setLayer(RenderLayer layer) {
        mLayer = layer;
    }

    void CRenderable::updateRenderableBuffer() {
        ObjectUniforms ub;
        ub.modelMatrix = mEntity->getComponent<CTransform>()->getWorldTransform();
        ub.normalMatrix = glm::mat4(1.0);
        mRenderableBuffer->update(&ub, 0, sizeof(ObjectUniforms));
    }

    glm::mat4 CRenderable::getWorldMatrix() {
        return mEntity->getComponent<CTransform>()->getWorldTransform();
    }

    void CRenderable::setCastShadow(bool castShadow) {
        for (int i = 0; i < mPrimitives.size(); ++i) {
            mPrimitives[i].castShadow = castShadow;
        }
    }

    void CRenderable::setReceiveShadow(bool receiveShadow) {
        for (int i = 0; i < mPrimitives.size(); ++i) {
            mPrimitives[i].receiveShadow = receiveShadow;
        }
    }

}