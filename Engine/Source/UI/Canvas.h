#pragma once
#include "Renderer/RenderData.h"
#include "View/View.h"
#include <GfxDefine.h>
#include <vector>
#include <memory>

namespace gear {
    class VertexBuffer;
    class IndexBuffer;
    class MaterialInstance;
    class Canvas {
    public:
        struct Element {
            uint32_t count;
            uint32_t offset;
            glm::vec4 scissor;
            blast::GfxTexture* texture = nullptr;
            View* view = nullptr;
        };

        struct Batch {
            uint8_t* vertex_data = nullptr;
            uint32_t vertex_count = 0;
            VertexLayoutType vertex_layout;
            uint8_t* index_data = nullptr;
            uint32_t index_count = 0;
            blast::IndexType index_type;
            std::vector<Element> elements;
        };

    public:
        Canvas();

        ~Canvas();

        void Begin();

        void End();

        void AddBatch(const Batch& batch);

    private:
        bool Prepare(blast::GfxCommandBuffer* cmd);

    private:
        friend class Renderer;
        std::vector<Batch> batchs;
        std::vector<std::shared_ptr<blast::GfxBuffer>> vertex_bufs;
        std::vector<std::shared_ptr<blast::GfxBuffer>> index_bufs;
        std::vector<UIDrawElement> draw_elements;
    };
}
