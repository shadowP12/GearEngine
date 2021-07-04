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

    // BEGIN: Shader里面的数据结构
    struct FrameUniforms {
        glm::mat4 viewMatrix;
        glm::mat4 projMatrix;
    };

    struct ObjectUniforms {
        glm::mat4 modelMatrix;
        glm::mat4 normalMatrix;
    };
    // END

    // 渲染层
    enum class RenderLayer {
        CORE = 0,
        DEBUG = 1,
        UI = 2,
    };

    struct RenderPrimitive {
        uint32_t offset = 0;
        uint32_t count = 0;
        BBox bbox;
        VertexBuffer* vertexBuffer = nullptr;
        IndexBuffer* indexBuffer = nullptr;
        MaterialInstance* materialInstance = nullptr;
        Blast::PrimitiveTopology type = Blast::PrimitiveTopology::PRIMITIVE_TOPO_TRI_STRIP;
    };

    struct Renderable {
        UniformBuffer* renderableUB = nullptr;
        UniformBuffer* boneUB = nullptr;
        std::vector<RenderPrimitive> primitives;
    };

    struct RenderView {
        UniformBuffer* cameraUB = nullptr;
        UniformBuffer* lightUB = nullptr;
        RenderTarget* renderTarget = nullptr;
        uint32_t renderableCount = 0;
        std::vector<Renderable> renderables;
        RenderLayer layer = RenderLayer::CORE;
        bool operator < (RenderView const& rhs) { return (uint32_t)layer < (uint32_t)rhs.layer; }
    };

    class RenderScene {
    public:
        RenderScene(Renderer* renderer);

        ~RenderScene();

        void prepare();
    private:
        friend Renderer;
        Renderer* mRenderer = nullptr;
        /**
         * 最多支持8个RenderView
         */
        uint32_t mViewCount = 0;
        RenderView mViews[8];
    };
}