#pragma once
#include "Core/GearDefine.h"
#include <Blast/Gfx/GfxDefine.h>

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

            void SetArray(uint32_t array);

            void SetFormat(blast::Format format);

            Texture* build();

        private:
            friend class Texture;
            uint32_t _width = 1;
            uint32_t _height = 1;
            uint32_t _depth = 1;
            uint32_t _array = 1;
            blast::Format _format;
        };

        ~Texture();

        void SetData(uint32_t i, void* data, uint32_t size);

        uint8_t* GetData(uint32_t i) { return _datas[i]; }

        blast::GfxTexture* GetTexture() { return _texture; }

    private:
        Texture(Builder* builder);

    private:
        friend class MaterialInstance;
        uint32_t _width;
        uint32_t _height;
        uint32_t _depth;
        uint32_t _array;
        blast::Format _format;
        // 后续将使用一块大的buffer代替vector
        std::vector<uint8_t*> _datas;
        blast::GfxTexture* _texture = nullptr;
    };

}