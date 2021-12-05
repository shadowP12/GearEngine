#include "Resource/Texture.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"

#include <Blast/Gfx/GfxDefine.h>
#include <Blast/Gfx/GfxDevice.h>

namespace gear {
    void Texture::Builder::SetWidth(uint32_t width) {
        this->width = width;
    }

    void Texture::Builder::SetHeight(uint32_t height) {
        this->height = height;
    }

    void Texture::Builder::SetDepth(uint32_t depth) {
        this->depth = depth;
    }

    void Texture::Builder::SetLayer(uint32_t layer) {
        this->layer = layer;
    }

    void Texture::Builder::SetLevel(uint32_t level) {
        this->level = level;
    }

    void Texture::Builder::SetFormat(blast::Format format) {
        this->format = format;
    }

    void Texture::Builder::SetCube(bool is_cube_map) {
        this->is_cube = is_cube_map;
    }

    Texture * Texture::Builder::Build() {
        return new Texture(this);
    }

    Texture::Texture(Builder* builder) {
        blast::ResourceUsage usage = blast::RESOURCE_USAGE_SHADER_RESOURCE;
        if (builder->is_cube && builder->layer == 6) {
            usage |= blast::RESOURCE_USAGE_TEXTURE_CUBE;
        }

        blast::GfxTextureDesc texture_desc;
        texture_desc.width = builder->width;
        texture_desc.height = builder->height;
        texture_desc.depth = builder->depth;
        texture_desc.num_layers = builder->layer;
        texture_desc.num_levels = builder->level;
        texture_desc.format = builder->format;
        texture_desc.res_usage = usage;
        texture_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
        texture = gEngine.GetDevice()->CreateTexture(texture_desc);
    }

    Texture::~Texture() {
        SAFE_DELETE_ARRAY(data);
        gEngine.GetDevice()->DestroyTexture(texture);
    }

    void Texture::UpdateData(void* data, uint32_t layer, uint32_t level) {
        uint32_t total_size = 0;
        for (uint32_t i = 0; i < level; ++i) {
            uint32_t image_size = 0;
            if (texture->desc.depth > 1) {
                image_size = texture->desc.width >> i;
                image_size *= texture->desc.height >> i;
                image_size *= texture->desc.depth >> i;
                image_size *= blast::GetFormatStride(texture->desc.format);
            } else if (texture->desc.height > 1) {
                image_size = texture->desc.width >> i;
                image_size *= texture->desc.height >> i;
                image_size *= blast::GetFormatStride(texture->desc.format);
            } else {
                image_size = texture->desc.width >> i;
                image_size *= blast::GetFormatStride(texture->desc.format);
            }

            total_size = image_size;
        }

        uint8_t* dst = this->data;
        uint8_t* src = (uint8_t*)data;
        if (data_size < total_size) {
            if (dst) {
                SAFE_DELETE_ARRAY(dst);
            }
            dst = new uint8_t[total_size];
            data_size = total_size;
        }
        memcpy(dst, src, total_size);

        blast::GfxDevice* device = gEngine.GetDevice();
        blast::GfxCommandBuffer* cmd = device->RequestCommandBuffer(blast::QUEUE_COPY);

        // 设置纹理为读写状态
        blast::GfxTextureBarrier barrier;
        barrier.texture = texture;
        barrier.new_state = blast::RESOURCE_STATE_COPY_DEST;
        device->SetBarrier(cmd, 0, nullptr, 1, &barrier);

        // 更新纹理数据
        device->UpdateTexture(cmd, texture, data);

        // 设置纹理为Shader可读状态
        barrier.texture = texture;
        barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(cmd, 0, nullptr, 1, &barrier);
    }
}