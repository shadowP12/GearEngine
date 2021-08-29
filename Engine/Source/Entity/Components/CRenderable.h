#pragma once
#include "Component.h"
#include "RenderPipeline/RenderPipeline.h"
#include "Math/Math.h"
#include <Blast/Gfx/GfxDefine.h>
#include <vector>

namespace gear {
    struct RenderPrimitive;
    class Entity;
    class VertexBuffer;
    class IndexBuffer;
    class UniformBuffer;
    class MaterialInstance;

    class CRenderable : public Component {
    public:
        CRenderable(Entity* entity);

        virtual ~CRenderable();

        static ComponentType GetClassType() { return ComponentType::Renderable; }

        ComponentType GetType() override { return ComponentType::Renderable; }

        void ResetPrimitives();

        void AddPrimitive(const RenderPrimitive& primitive);

        void SetVertexBuffer(VertexBuffer* buffer);

        void SetIndexBuffer(IndexBuffer* buffer);

        void SetCastShadow(bool castShadow);

        void SetReceiveShadow(bool receiveShadow);

        RenderableType GetRenderableType() { return _renderable_type; }

        void SetRenderableType(RenderableType type) { _renderable_type = type; }

    private:
        friend class RenderPipeline;
        RenderableType _renderable_type = RENDERABLE_COMMON;
        VertexBuffer* _vb = nullptr;
        IndexBuffer* _ib = nullptr;
        UniformBuffer* _bb = nullptr;
        std::vector<RenderPrimitive> _primitives;
    };
}