#pragma once

#include "Core/GearDefine.h"
#include "Resource/Resource.h"
#include <Blast/Gfx/GfxDefine.h>

namespace Blast {
    class GfxTexture;
}

namespace gear {
    /*
     * TODO: 提供生成mipmap接口
     */
    class Texture : public Resource {
    public:
        class Builder {
        public:
            Builder() = default;

            ~Builder() = default;

            void width(uint32_t width);

            void height(uint32_t height);

            void depth(uint32_t depth);

            void array(uint32_t array);

            void format(Blast::Format format);

            Texture* build();

        private:
            friend class Texture;
            uint32_t mWidth = 1;
            uint32_t mHeight = 1;
            uint32_t mDepth = 1;
            uint32_t mArray = 1;
            Blast::Format mFormat;
        };

        ~Texture();

        // 设置cpu数据的时候，自动同步到gpu
        void setData(uint32_t i, void* data, uint32_t size);

        uint8_t* getData(uint32_t i) { return mDatas[i]; }

        Blast::GfxTexture* getTexture() { return mInternelTexture; }

    private:
        Texture(Builder* builder);

    private:
        friend class MaterialInstance;
        ResourceType mType = ResourceType::TEXTURE;
        uint32_t mWidth;
        uint32_t mHeight;
        uint32_t mDepth;
        uint32_t mArray;
        Blast::Format mFormat;
        // 后续将使用一块大的buffer代替vec
        std::vector<uint8_t*> mDatas;
        Blast::GfxTexture* mInternelTexture = nullptr;
    };

}