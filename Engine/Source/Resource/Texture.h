#pragma once
#include "Core/GearDefine.h"
#include <Blast/Gfx/GfxDefine.h>
#include <vector>

namespace blast {
    class GfxTexture;
}

namespace gear {
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

            void SetCube(bool is_cube);

            Texture* Build();

        private:
            friend class Texture;
            uint32_t width = 1;
            uint32_t height = 1;
            uint32_t depth = 1;
            uint32_t layer = 1;
            uint32_t level = 1;
            bool is_cube = false;
            blast::Format format;
        };

        ~Texture();

        void UpdateData(void* data, uint32_t layer = 0, uint32_t level = 0);

        uint8_t* GetData() { return data; }

        blast::GfxTexture* GetTexture() { return texture; }

    private:
        Texture(Builder* builder);

    private:
        friend class MaterialInstance;
        uint8_t* data = nullptr;
        uint32_t data_size = 0;
        blast::GfxTexture* texture = nullptr;
    };

}