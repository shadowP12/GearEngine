#pragma once

#include "Core/GearDefine.h"
#include "Resource/Resource.h"

#include <Blast/Gfx/GfxDefine.h>

namespace Blast {
    class GfxTexture;
}

namespace gear {
    struct TextureDesc {
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        uint32_t array = 1;
        Blast::Format format;
    };

    /*
     * TODO: 提供生成mipmap接口
     */
    class Texture : public Resource {
    public:
        Texture(const TextureDesc& desc);
        ~Texture();
        void setData(uint32_t i, void* data, uint32_t size);
        uint8_t* getData(uint32_t i) { return mDatas[i]; }
        Blast::GfxTexture* getInternel() { return mInternelTexture; }
        void updateRenderData();
    private:
        friend class MaterialInstance;
        ResourceType mType = ResourceType::TEXTURE;
        uint32_t mWidth;
        uint32_t mHeight;
        uint32_t mDepth;
        uint32_t mArray;
        Blast::Format mFormat;
        std::vector<uint8_t*> mDatas;
        Blast::GfxTexture* mInternelTexture = nullptr;
    };

}