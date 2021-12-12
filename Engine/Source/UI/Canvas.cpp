#include "Canvas.h"
#include "GearEngine.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"

#include <Blast/Gfx/GfxDevice.h>

namespace gear {
    Canvas::Canvas() {
    }

    Canvas::~Canvas() {
        for (int i = 0; i < vbs.size(); ++i) {
            SAFE_DELETE(vbs[i]);
        }
        vbs.clear();

        for (int i = 0; i < ibs.size(); ++i) {
            SAFE_DELETE(ibs[i]);
        }
        ibs.clear();
    }

    void Canvas::Begin() {
        batchs.clear();
    }

    void Canvas::End() {
    }

    void Canvas::AddBatch(const Batch& batch) {
        batchs.push_back(batch);
    }

    bool Canvas::Prepare(blast::GfxCommandBuffer* cmd) {
        draw_elements.clear();

        if (batchs.size() > vbs.size()) {
            uint32_t previous_size = vbs.size();
            vbs.resize(batchs.size(), nullptr);
            for (uint32_t i = previous_size; i < vbs.size(); i++) {
                gear::VertexBuffer::Builder builder;
                builder.SetVertexCount(10000);
                builder.SetVertexLayoutType(batchs[i].vertex_layout);
                vbs[i] = builder.Build();
            }
        }

        if (batchs.size() > ibs.size()) {
            uint32_t previous_size = ibs.size();
            ibs.resize(batchs.size(), nullptr);
            for (uint32_t i = previous_size; i < ibs.size(); i++) {
                gear::IndexBuffer::Builder builder;
                builder.SetIndexCount(5000);
                builder.SetIndexType(batchs[i].index_type);
                ibs[i] = builder.Build();
            }
        }

        for (uint32_t i = 0; i < batchs.size(); ++i) {
            // 更新顶点/索引缓存
            uint32_t vb_size = batchs[i].vertex_count * GetVertexLayoutStride(batchs[i].vertex_layout);
            if (vbs[i]->GetSize() < vb_size) {
                SAFE_DELETE(vbs[i]);
                uint32_t require_count = batchs[i].vertex_count * 2;
                gear::VertexBuffer::Builder builder;
                builder.SetVertexCount(require_count);
                builder.SetVertexLayoutType(batchs[i].vertex_layout);
                vbs[i] = builder.Build();
            }
            gEngine.GetDevice()->UpdateBuffer(cmd, vbs[i]->GetHandle(), batchs[i].vertex_data, vb_size);

            uint32_t ib_size = batchs[i].index_type == blast::INDEX_TYPE_UINT32 ? batchs[i].index_count * sizeof(uint32_t) : batchs[i].index_count * sizeof(uint16_t);
            if (ibs[i]->GetSize() < ib_size) {
                SAFE_DELETE(ibs[i]);
                uint32_t require_count = batchs[i].index_count * 2;
                gear::IndexBuffer::Builder builder;
                builder.SetIndexCount(require_count);
                builder.SetIndexType(batchs[i].index_type);
                ibs[i] = builder.Build();
            }
            gEngine.GetDevice()->UpdateBuffer(cmd, ibs[i]->GetHandle(), batchs[i].index_data, ib_size);

            for (uint32_t j = 0; j < batchs[i].elements.size(); ++j) {
                UIDrawElement draw_element = {};
                draw_element.vb = vbs[i];
                draw_element.ib = ibs[i];
                draw_element.mi = batchs[i].elements[j].mi;
                draw_element.count = batchs[i].elements[j].count;
                draw_element.offset = batchs[i].elements[j].offset;
                draw_elements.push_back(draw_element);
            }
        }

        return true;
    }
}