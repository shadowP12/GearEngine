#pragma once
#include "Core/GearDefine.h"
#include "RenderTarget.h"
#include "Math/Math.h"
#include <Blast/Gfx/GfxBuffer.h>
#include <Blast/Gfx/GfxTexture.h>
#include <Blast/Gfx/GfxSampler.h>
#include <Blast/Gfx/GfxShader.h>
#include <map>
#include <vector>

namespace gear {
    class VertexBuffer;
    class IndexBuffer;
    class UniformBuffer;
    class MaterialInstance;
    class RenderTarget;

    // TODO: 包围盒
    struct Renderable {
        VertexBuffer* vertexBuffer = nullptr;
        IndexBuffer* indexBuffer = nullptr;
        UniformBuffer* renderableUB = nullptr;
        UniformBuffer* boneUB = nullptr;
        MaterialInstance* materialInstance = nullptr;
        Blast::PrimitiveTopology type = Blast::PrimitiveTopology::PRIMITIVE_TOPO_TRI_STRIP;
        uint32_t offset = 0;
        uint32_t count = 0;
    };

    struct RenderView {
        UniformBuffer* cameraUB = nullptr;
        UniformBuffer* lightUB = nullptr;
        RenderTarget* renderTarget = nullptr;
    };

    class RenderScene {
    public:
        RenderScene(Renderer* renderer);

        ~RenderScene();

        void prepare();
    private:
        friend Renderer;
        Renderer* mRenderer = nullptr;
        uint32_t mViewCount = 0;
        std::vector<RenderView> mViews;
        uint32_t mRenderableCount = 0;
        std::vector<Renderable> mRenderables;
    };
}