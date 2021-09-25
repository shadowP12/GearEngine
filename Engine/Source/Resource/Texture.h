#pragma once
#include "Core/GearDefine.h"
#include <Blast/Gfx/GfxDefine.h>
#include <vector>

namespace blast {
    class GfxTexture;
}

namespace gear {
    //TODO: 提供生成mipmap接口

    class Texture {
    public:
        class Builder {
        public:
            Builder() = default;

            ~Builder() = default;

            void SetWidth(uint32_t width);

            void SetHeight(uint32_t height);

            void SetDepth(uint32_t depth);

            void SetLayer(uint32_t layer);

            void SetLevel(uint32_t level);

            void SetFormat(blast::Format format);

            Texture* Build();

        private:
            friend class Texture;
            uint32_t _width = 1;
            uint32_t _height = 1;
            uint32_t _depth = 1;
            uint32_t _layer = 1;
            uint32_t _level = 1;
            blast::Format _format;
        };

        ~Texture();

        void SetData(void* data);

        void SetData(void* data, uint32_t layer, uint32_t level);

        uint8_t* GetData() { return _data; }

        blast::GfxTexture* GetTexture() { return _texture; }

    private:
        Texture(Builder* builder);

    private:
        friend class MaterialInstance;
        uint32_t _width;
        uint32_t _height;
        uint32_t _depth;
        uint32_t _layer;
        uint32_t _level;
        blast::Format _format;
        // 后续将使用一块大的buffer代替vector
        uint8_t* _data = nullptr;
        uint32_t _data_size = 0;
        blast::GfxTexture* _texture = nullptr;
    };

}