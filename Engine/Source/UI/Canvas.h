#pragma once
#include "Renderer/RenderData.h"

#include <Blast/Gfx/GfxDefine.h>

#include <vector>

namespace gear {
    class VertexBuffer;
    class IndexBuffer;
    class MaterialInstance;
    class Canvas {
    public:
        struct Element {
            uint32_t count;
            uint32_t offset;
            MaterialInstance* mi;
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
        std::vector<VertexBuffer*> vbs;
        std::vector<IndexBuffer*> ibs;
        std::vector<UIDrawElement> draw_elements;
    };
}
