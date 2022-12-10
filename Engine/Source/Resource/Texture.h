#pragma once
#include "Core/GearDefine.h"
#include <GfxDefine.h>
#include <vector>
#include <memory>

namespace gear {
    class TextureData {
    public:
        class Builder {
        public:
            Builder() = default;

            ~Builder() = default;

            Builder& SetWidth(uint32_t width);

            Builder& SetHeight(uint32_t height);

            Builder& SetDepth(uint32_t depth);

            Builder& SetLayer(uint32_t layer);

            Builder& SetLevel(uint32_t level);

            Builder& SetFormat(blast::Format format);

            Builder& SetCube(bool is_cube);

            Builder& SetData(uint8_t* data, uint32_t data_size);

            std::shared_ptr<TextureData> Build();

        private:
            friend class TextureData;
            uint32_t width = 1;
            uint32_t height = 1;
            uint32_t depth = 1;
            uint32_t layer = 1;
            uint32_t level = 1;
            bool is_cube = false;
            blast::Format format;
            uint32_t data_size;
            uint8_t* data = nullptr;
        };

        ~TextureData();

        std::shared_ptr<blast::GfxTexture> LoadTexture();

    private:
        TextureData(Builder* builder);

    public:
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        uint32_t layer = 1;
        uint32_t level = 1;
        bool is_cube = false;
        blast::Format format;
        uint32_t data_size;
        uint8_t* data = nullptr;
    };
}