#pragma once
#include "Scene/Components/Component.h"
#include "Renderer/RenderScene.h"
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

        static ComponentType getClassType() { return ComponentType::Renderable; }

        ComponentType getType() override { return ComponentType::Renderable; }

        void resetPrimitives();

        void addPrimitive(const RenderPrimitive& primitive);

        void setVertexBuffer(VertexBuffer* buffer);

        void setIndexBuffer(IndexBuffer* buffer);

        void setLayer(RenderLayer layer);

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
        std::vector<RenderPrimitive> mPrimitives;
        RenderLayer mLayer = RenderLayer::CORE;
    };
}