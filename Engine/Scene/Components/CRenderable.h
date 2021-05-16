#pragma once
#include "Scene/Components/Component.h"
#include "Math/Math.h"

namespace gear {
    class Entity;
    class VertexBuffer;
    class IndexBuffer;
    class MaterialInstance;
    class CRenderable : public Component {
    public:
        CRenderable(Entity* entity);

        virtual ~CRenderable();

        static ComponentType getClassType() { return ComponentType::Renderable; }

        ComponentType getType() override { return ComponentType::Renderable; }

        void setVertexBuffer(VertexBuffer* buffer);

        void setIndexBuffer(IndexBuffer* buffer);

        void setMaterialInstance(MaterialInstance* instance);
    private:
        friend class Renderer;
        VertexBuffer* mVertexBuffer = nullptr;
        IndexBuffer* mIndexBufferr = nullptr;
        MaterialInstance* mMaterialInstance = nullptr;
    };
}