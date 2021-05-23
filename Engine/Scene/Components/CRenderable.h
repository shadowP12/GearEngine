#pragma once
#include "Scene/Components/Component.h"
#include "Math/Math.h"
#include <Blast/Gfx/GfxDefine.h>

namespace gear {
    class Entity;
    class VertexBuffer;
    class IndexBuffer;
    class UniformBuffer;
    class MaterialInstance;

    class RenderPrimitive {
    public:
        RenderPrimitive(Blast::PrimitiveTopology type, uint32_t offset, uint32_t count)
        : type(type), offset(offset), count(count) {
        }

        RenderPrimitive() = default;

        ~RenderPrimitive() = default;
    public:
        Blast::PrimitiveTopology type = Blast::PrimitiveTopology::PRIMITIVE_TOPO_TRI_STRIP;
        uint32_t offset = 0;
        uint32_t count = 0;
    };

    class CRenderable : public Component {
    public:
        CRenderable(Entity* entity);

        virtual ~CRenderable();

        static ComponentType getClassType() { return ComponentType::Renderable; }

        ComponentType getType() override { return ComponentType::Renderable; }

        void setPrimitive(RenderPrimitive primitive);

        void setVertexBuffer(VertexBuffer* buffer);

        void setIndexBuffer(IndexBuffer* buffer);

        void setMaterialInstance(MaterialInstance* instance);
    private:
        void updateRenderableBuffer();
    private:
        friend class Renderer;
        friend class RenderScene;
        VertexBuffer* mVertexBuffer = nullptr;
        IndexBuffer* mIndexBufferr = nullptr;
        // 每一个Renderable必备的UniformBuffer
        UniformBuffer* mRenderableBuffer = nullptr;
        // TODO: 后续添加骨骼动画
        UniformBuffer* mBoneBuffer = nullptr;
        MaterialInstance* mMaterialInstance = nullptr;
        RenderPrimitive mPrimitive;
    };
}