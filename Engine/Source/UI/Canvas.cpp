#include "Canvas.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderCache.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include <GfxDevice.h>

namespace gear {
    Canvas::Canvas() {
    }

    Canvas::~Canvas() {
        vertex_bufs.clear();
        index_bufs.clear();
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
        Renderer* renderer = gEngine.GetRenderer();
        blast::GfxDevice* device = renderer->GetDevice();
        blast::GfxSamplerDesc sampler_desc;
        blast::GfxSampler* sampler = renderer->GetSamplerCache()->GetSampler(sampler_desc);

        draw_elements.clear();

        if (batchs.size() > vertex_bufs.size()) {
            uint32_t previous_size = vertex_bufs.size();
            vertex_bufs.resize(batchs.size(), nullptr);
            for (uint32_t i = previous_size; i < vertex_bufs.size(); i++) {
                blast::GfxBufferDesc buffer_desc{};
                buffer_desc.size = 20 * 2048;
                buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
                buffer_desc.res_usage = blast::RESOURCE_USAGE_VERTEX_BUFFER;
                vertex_bufs[i] = std::shared_ptr<blast::GfxBuffer>(device->CreateBuffer(buffer_desc));
            }
        }

        if (batchs.size() > index_bufs.size()) {
            uint32_t previous_size = index_bufs.size();
            index_bufs.resize(batchs.size(), nullptr);
            for (uint32_t i = previous_size; i < index_bufs.size(); i++) {
                blast::GfxBufferDesc buffer_desc{};
                buffer_desc.size = 10240;
                buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
                buffer_desc.res_usage = blast::RESOURCE_USAGE_INDEX_BUFFER;
                index_bufs[i] = std::shared_ptr<blast::GfxBuffer>(device->CreateBuffer(buffer_desc));
            }
        }

        for (uint32_t i = 0; i < batchs.size(); ++i) {
            // 更新顶点/索引缓存
            uint32_t vb_size = batchs[i].vertex_count * GetVertexLayoutStride(batchs[i].vertex_layout);
            if (vertex_bufs[i]->size < vb_size) {
                vertex_bufs[i].reset();
                uint32_t require_size = vb_size * 2;
                blast::GfxBufferDesc buffer_desc{};
                buffer_desc.size = require_size;
                buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
                buffer_desc.res_usage = blast::RESOURCE_USAGE_VERTEX_BUFFER;
                vertex_bufs[i] = std::shared_ptr<blast::GfxBuffer>(device->CreateBuffer(buffer_desc));
            }
            renderer->UpdateVertexBuffer(cmd, vertex_bufs[i].get(), batchs[i].vertex_data, vb_size, 0);

            uint32_t ib_size = batchs[i].index_type == blast::INDEX_TYPE_UINT32 ? batchs[i].index_count * sizeof(uint32_t) : batchs[i].index_count * sizeof(uint16_t);
            if (index_bufs[i]->size < ib_size) {
                index_bufs[i].reset();
                uint32_t require_size = ib_size * 2;
                blast::GfxBufferDesc buffer_desc{};
                buffer_desc.size = require_size;
                buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
                buffer_desc.res_usage = blast::RESOURCE_USAGE_INDEX_BUFFER;
                index_bufs[i] = std::shared_ptr<blast::GfxBuffer>(device->CreateBuffer(buffer_desc));
            }
            renderer->UpdateIndexBuffer(cmd, index_bufs[i].get(), batchs[i].index_data, ib_size, 0);

            for (uint32_t j = 0; j < batchs[i].elements.size(); ++j) {
                UIDrawElement draw_element = {};
                draw_element.vb = vertex_bufs[i].get();
                draw_element.ib = index_bufs[i].get();
                draw_element.index_count = batchs[i].elements[j].count;
                draw_element.index_offset = batchs[i].elements[j].offset;
                draw_element.index_type = batchs[i].index_type;
                draw_element.scissor = batchs[i].elements[j].scissor;
                draw_element.sampler = sampler;
                draw_element.texture = batchs[i].elements[j].texture;
                draw_elements.push_back(draw_element);
            }
        }

        return true;
    }
}