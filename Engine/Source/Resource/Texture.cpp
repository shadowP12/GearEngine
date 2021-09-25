#include "Resource/Texture.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include <Blast/Gfx/GfxContext.h>
#include <Blast/Gfx/GfxBuffer.h>
#include <Blast/Gfx/GfxTexture.h>
#include <Blast/Gfx/GfxCommandBuffer.h>

namespace gear {
    void Texture::Builder::SetWidth(uint32_t width) {
        _width = width;
    }

    void Texture::Builder::SetHeight(uint32_t height) {
        _height = height;
    }

    void Texture::Builder::SetDepth(uint32_t depth) {
        _depth = depth;
    }

    void Texture::Builder::SetLayer(uint32_t layer) {
        _layer = layer;
    }

    void Texture::Builder::SetLevel(uint32_t level) {
        _level = level;
    }

    void Texture::Builder::SetFormat(blast::Format format) {
        _format = format;
    }

    Texture * Texture::Builder::Build() {
        return new Texture(this);
    }

    Texture::Texture(Builder* builder) {
        _width = builder->_width;
        _height = builder->_height;
        _depth = builder->_depth;
        _layer = builder->_layer;
        _level = builder->_level;
        _format = builder->_format;

        // 计算cpu data size
        uint32_t total_image_size = 0;
        for (uint32_t i = 0; i < _layer; ++i) {
            for (uint32_t j = 0; j < _level; ++j) {
                uint32_t image_size = 0;
                if (_depth > 1)
                {
                    image_size = _width >> j;
                    image_size *= _height >> j;
                    image_size *= _depth >> j;
                    image_size *= blast::GetFormatStride(_format);

                } else if (_height > 1)
                {
                    image_size = _width >> j;
                    image_size *= _height >> j;
                    image_size *= blast::GetFormatStride(_format);
                } else {
                    image_size = _width >> j;
                    image_size *= blast::GetFormatStride(_format);
                }
                total_image_size += image_size;
            }
        }
        _data_size = total_image_size;
        _data = new uint8_t[total_image_size];

        blast::GfxContext* context = gEngine.GetRenderer()->GetContext();
        blast::GfxTextureDesc texture_desc;
        texture_desc.width = _width;
        texture_desc.height = _height;
        texture_desc.depth = _depth;
        texture_desc.num_layers = _layer;
        texture_desc.num_mips = _level;
        texture_desc.format = _format;
        texture_desc.type = blast::RESOURCE_TYPE_TEXTURE;
        texture_desc.usage = blast::RESOURCE_USAGE_GPU_ONLY;
        _texture = context->CreateTexture(texture_desc);
    }

    Texture::~Texture() {
        SAFE_DELETE(_data);

        Renderer* renderer = gEngine.GetRenderer();
        renderer->Destroy(_texture);
    }

    void Texture::SetData(void* data, uint32_t layer, uint32_t level) {
        Renderer* renderer = gEngine.GetRenderer();
        uint32_t layer_offset = _data_size / _layer * layer;
        uint32_t level_offset = 0;
        uint32_t total_size = 0;
        for (uint32_t i = 0; i < level; ++i) {
            uint32_t image_size = 0;
            if (_depth > 1)
            {
                image_size = _width >> i;
                image_size *= _height >> i;
                image_size *= _depth >> i;
                image_size *= blast::GetFormatStride(_format);

            } else if (_height > 1)
            {
                image_size = _width >> i;
                image_size *= _height >> i;
                image_size *= blast::GetFormatStride(_format);
            } else {
                image_size = _width >> i;
                image_size *= blast::GetFormatStride(_format);
            }

            total_size = image_size;
            level_offset += image_size;
        }
        uint32_t offset = layer_offset + level_offset;

        uint8_t* dst = _data + offset;
        uint8_t* src = (uint8_t*)data;
        memcpy(dst, src, total_size);

        blast::GfxTexture* texture = _texture;
        blast::GfxBuffer* staging_buffer = renderer->AllocStageBuffer(total_size);
        staging_buffer->WriteData(0, total_size, data);

        renderer->ExecRenderTask([renderer, texture, staging_buffer, layer, level](blast::GfxCommandBuffer* cmd) {

            renderer->UseResource(texture);
            renderer->UseResource(staging_buffer);

            {
                // 设置纹理为读写状态
                blast::GfxTextureBarrier barrier;
                barrier.texture = texture;
                barrier.new_state = blast::RESOURCE_STATE_COPY_DEST;
                cmd->SetBarrier(0, nullptr, 1, &barrier);
            }

            blast::GfxCopyToImageRange range;
            range.buffer_offset = 0;
            range.layer = layer;
            range.level = level;
            cmd->CopyToImage(staging_buffer, texture, range);

            {
                // 设置纹理为Shader可读状态
                blast::GfxTextureBarrier barrier;
                barrier.texture = texture;
                barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
                cmd->SetBarrier(0, nullptr, 1, &barrier);
            }
        });
    }

    void Texture::SetData(void* data) {
        Renderer* renderer = gEngine.GetRenderer();
        uint8_t* dst = _data;
        uint8_t* src = (uint8_t*)data;
        memcpy(dst, src, _data_size);

        blast::GfxTexture* texture = _texture;
        blast::GfxBuffer* staging_buffer = renderer->AllocStageBuffer(_data_size);

        uint32_t offset = 0;
        for (uint32_t i = 0; i < _layer; ++i) {
            for (uint32_t j = 0; j < _level; ++j) {
                uint32_t image_size = 0;
                if (_depth > 1)
                {
                    image_size = _width >> j;
                    image_size *= _height >> j;
                    image_size *= _depth >> j;
                    image_size *= blast::GetFormatStride(_format);

                } else if (_height > 1)
                {
                    image_size = _width >> j;
                    image_size *= _height >> j;
                    image_size *= blast::GetFormatStride(_format);
                } else {
                    image_size = _width >> j;
                    image_size *= blast::GetFormatStride(_format);
                }
                staging_buffer->WriteData(offset, image_size, _data);
                offset += image_size;
            }
        }

        uint32_t layer = _layer;
        uint32_t level = _level;
        renderer->ExecRenderTask([renderer, texture, staging_buffer, layer, level](blast::GfxCommandBuffer* cmd) {

            renderer->UseResource(texture);
            renderer->UseResource(staging_buffer);

            uint32_t width = texture->GetWidth();
            uint32_t height = texture->GetHeight();
            uint32_t depth = texture->GetDepth();
            blast::Format format = texture->GetFormat();

            {
                // 设置纹理为读写状态
                blast::GfxTextureBarrier barrier;
                barrier.texture = texture;
                barrier.new_state = blast::RESOURCE_STATE_COPY_DEST;
                cmd->SetBarrier(0, nullptr, 1, &barrier);
            }

            blast::GfxCopyToImageRange range;
            uint32_t offset = 0;
            for (uint32_t i = 0; i < layer; ++i) {
                for (uint32_t j = 0; j < level; ++j) {
                    uint32_t image_size = 0;
                    if (depth > 1)
                    {
                        image_size = width >> j;
                        image_size *= height >> j;
                        image_size *= depth >> j;
                        image_size *= blast::GetFormatStride(format);

                    } else if (height > 1)
                    {
                        image_size = width >> j;
                        image_size *= height >> j;
                        image_size *= blast::GetFormatStride(format);
                    } else {
                        image_size = width >> j;
                        image_size *= blast::GetFormatStride(format);
                    }
                    range.buffer_offset = offset;
                    range.layer = i;
                    range.level = j;
                    cmd->CopyToImage(staging_buffer, texture, range);
                    offset += image_size;
                }
            }

            {
                // 设置纹理为Shader可读状态
                blast::GfxTextureBarrier barrier;
                barrier.texture = texture;
                barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
                cmd->SetBarrier(0, nullptr, 1, &barrier);
            }
        });
    }
}