#pragma once
#include "Core/GearDefine.h"
#include "Renderer/RenderData.h"
#include "Math/Math.h"
#include <GfxDefine.h>
#include <vector>

namespace gear {
    class Scene;
    class BaseWindow;
    class MaterialInstance;

    class View {
    public:
        View();

        ~View();

        void SetSize(float w, float h);

        void SetViewport(float x, float y, float w, float h);

        void SetSampleCount(blast::SampleCount sm);

    private:
        bool Prepare(blast::GfxCommandBuffer* cmd);

        bool ResizeBuffers();

        void SwapPostProcess();

        blast::GfxTexture* GetInPostProcessRT();

        blast::GfxTexture* GetOutPostProcessRT();

    private:
        friend class Renderer;
        BaseWindow* window = nullptr;
        glm::vec2 size;
        glm::vec2 old_size;
        glm::vec4 viewport;
        blast::SampleCount sample_count = blast::SAMPLE_COUNT_1;
        blast::SampleCount old_sample_count = blast::SAMPLE_COUNT_1;
        // MainRT支持多采样
        blast::GfxTexture* main_rt = nullptr;
        blast::GfxTexture* main_resolve_rt = nullptr;
        // DepthRT支持多采样
        blast::GfxTexture* depth_rt = nullptr;
        blast::GfxTexture* depth_resolve_rt = nullptr;
        blast::GfxRenderPass* renderpass = nullptr;

        // 后处理(采用ping-pong的方法进行后处理)
        uint32_t out_postprocess_idx = 0;
        blast::GfxTexture* postprocess_rt0 = nullptr;
        blast::GfxTexture* postprocess_rt1 = nullptr;
    };
}
