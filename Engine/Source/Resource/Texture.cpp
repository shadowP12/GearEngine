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

    void Texture::Builder::SetArray(uint32_t array) {
        _array = array;
    }

    void Texture::Builder::SetFormat(blast::Format format) {
        _format = format;
    }

    Texture * Texture::Builder::build() {
        return new Texture(this);
    }

    Texture::Texture(Builder* builder) {
        _width = builder->_width;
        _height = builder->_height;
        _depth = builder->_depth;
        _array = builder->_array;
        _format = builder->_format;
        for (int i = 0; i < _array; ++i) {
            uint8_t* data = new uint8_t[_width * _height * _depth * blast::GetFormatStride(_format)];
            _datas.push_back(data);
        }

        blast::GfxContext* context = gEngine.GetRenderer()->GetContext();
        blast::GfxTextureDesc texture_desc;
        texture_desc.width = _width;
        texture_desc.height = _height;
        texture_desc.format = _format;
        texture_desc.type = blast::RESOURCE_TYPE_TEXTURE | blast::RESOURCE_TYPE_RW_TEXTURE;
        texture_desc.usage = blast::RESOURCE_USAGE_GPU_ONLY;
        _texture = context->CreateTexture(texture_desc);
    }

    Texture::~Texture() {
        for (int i = 0; i < _datas.size(); ++i) {
            SAFE_DELETE_ARRAY(_datas[i]);
        }
        _datas.clear();

        Renderer* renderer = gEngine.GetRenderer();
        renderer->Destroy(_texture);
    }

    void Texture::SetData(uint32_t i, void* data, uint32_t size) {
        uint8_t* dst = _datas[i];
        uint8_t* src = (uint8_t*)data;
        memcpy(dst, src, size);

        Renderer* renderer = gEngine.GetRenderer();
        renderer->ExecRenderTask([this, renderer](blast::GfxCommandBuffer* cmd) {
            uint32_t image_size = _width * _height * _depth * blast::GetFormatStride(_format);
            uint32_t total_size = image_size * _array;
            blast::GfxBuffer* staging_buffer = renderer->AllocStageBuffer(total_size);
            uint32_t offset = 0;
            for (int i = 0; i < _array; ++i) {
                staging_buffer->WriteData(offset, image_size, _datas[i]);
                offset += image_size;
            }
            renderer->UseResource(staging_buffer);

            {
                // 设置纹理为读写状态
                blast::GfxTextureBarrier barrier;
                barrier.texture = _texture;
                barrier.new_state = blast::RESOURCE_STATE_COPY_DEST;
                cmd->SetBarrier(0, nullptr, 1, &barrier);
            }

            offset = 0;
            blast::GfxCopyToImageRange range;
            range.buffer_offset = 0;
            range.layer = 0;
            range.level = 0;
            for (int i = 0; i < _array; ++i) {
                range.buffer_offset = offset;
                range.layer = i;
                range.level = 0;
                cmd->CopyToImage(staging_buffer, _texture, range);
                offset += image_size;
            }

            {
                // 设置纹理为Shader可读状态
                blast::GfxTextureBarrier barrier;
                barrier.texture = _texture;
                barrier.new_state = blast::RESOURCE_STATE_SHADER_RESOURCE;
                cmd->SetBarrier(0, nullptr, 1, &barrier);
            }
        });
    }
}