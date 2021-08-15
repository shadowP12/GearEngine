#include "CRenderable.h"
#include "CTransform.h"
#include "Entity/Entity.h"
#include "Renderer/Renderer.h"
#include "Resource/GpuBuffer.h"

namespace gear {
    CRenderable::CRenderable(Entity* entity)
            :Component(entity) {
    }

    CRenderable::~CRenderable() {
    }

    void CRenderable::ResetPrimitives() {
        _primitives.clear();
    }

    void CRenderable::AddPrimitive(const RenderPrimitive& primitive) {
        _primitives.push_back(primitive);
    }

    void CRenderable::SetVertexBuffer(VertexBuffer* buffer) {
        _vb = buffer;
    }

    void CRenderable::SetIndexBuffer(IndexBuffer* buffer) {
        _ib = buffer;
    }

    void CRenderable::SetCastShadow(bool cast_shadow) {
        for (int i = 0; i < _primitives.size(); ++i) {
            _primitives[i].cast_shadow = cast_shadow;
        }
    }

    void CRenderable::SetReceiveShadow(bool receive_shadow) {
        for (int i = 0; i < _primitives.size(); ++i) {
            _primitives[i].receive_shadow = receive_shadow;
        }
    }

}