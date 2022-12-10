#include "Resource/Texture.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"

#include <GfxDefine.h>
#include <GfxDevice.h>

namespace gear {
    TextureData::Builder& TextureData::Builder::SetWidth(uint32_t width) {
        this->width = width;
        return *this;
    }

    TextureData::Builder& TextureData::Builder::SetHeight(uint32_t height) {
        this->height = height;
        return *this;
    }

    TextureData::Builder& TextureData::Builder::SetDepth(uint32_t depth) {
        this->depth = depth;
        return *this;
    }

    TextureData::Builder& TextureData::Builder::SetLayer(uint32_t layer) {
        this->layer = layer;
        return *this;
    }

    TextureData::Builder& TextureData::Builder::SetLevel(uint32_t level) {
        this->level = level;
        return *this;
    }

    TextureData::Builder& TextureData::Builder::SetFormat(blast::Format format) {
        this->format = format;
        return *this;
    }

    TextureData::Builder& TextureData::Builder::SetCube(bool is_cube_map) {
        this->is_cube = is_cube_map;
        return *this;
    }

    TextureData::Builder& TextureData::Builder::SetData(uint8_t* data, uint32_t data_size) {
        this->data = new uint8_t[data_size];
        memcpy(this->data, data, data_size);
        this->data_size = data_size;
        return *this;
    }

    std::shared_ptr<TextureData> TextureData::Builder::Build() {
        return std::shared_ptr<TextureData>(new TextureData(this));
    }

    TextureData::TextureData(Builder* builder) {
        width = builder->width;
        height = builder->height;
        depth = builder->depth;
        layer = builder->layer;
        level = builder->level;
        is_cube = builder->is_cube;
        format = builder->format;
        data_size = builder->data_size;
        data = builder->data;
    }

    TextureData::~TextureData() {
        SAFE_DELETE_ARRAY(data);
    }

    std::shared_ptr<blast::GfxTexture> TextureData::LoadTexture() {
        blast::ResourceUsage usage = blast::RESOURCE_USAGE_SHADER_RESOURCE | blast::RESOURCE_USAGE_RENDER_TARGET | blast::RESOURCE_USAGE_UNORDERED_ACCESS;
        if (is_cube && layer == 6) {
            usage |= blast::RESOURCE_USAGE_CUBE_TEXTURE;
        }

        blast::GfxTextureDesc texture_desc;
        texture_desc.width = width;
        texture_desc.height = height;
        texture_desc.depth = depth;
        texture_desc.num_layers = layer;
        texture_desc.num_levels = level;
        texture_desc.format = format;
        texture_desc.res_usage = usage;
        texture_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
        blast::GfxTexture* texture = gEngine.GetRenderer()->GetDevice()->CreateTexture(texture_desc);

        blast::GfxDevice* device = gEngine.GetRenderer()->GetDevice();
        blast::GfxCommandBuffer* cmd = device->RequestCommandBuffer(blast::QUEUE_COPY);

        blast::GfxResourceBarrier barrier;
        barrier.resource = texture;
        barrier.new_state = blast::RESOURCE_STATE_COPY_DEST;
        device->SetBarrier(cmd, 1, &barrier);

        device->UpdateTexture(cmd, texture, data);

        barrier.resource = texture;
        barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
        device->SetBarrier(cmd, 1, &barrier);

        return std::shared_ptr<blast::GfxTexture>(texture);
    }
}