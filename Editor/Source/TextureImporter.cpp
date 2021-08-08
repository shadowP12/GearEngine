#include "TextureImporter.h"
#include <Engine/Resource/Texture.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBIR_FLAG_ALPHA_PREMULTIPLIED
#include <stb_image.h>

TextureImporter::TextureImporter() {
}

TextureImporter::~TextureImporter() {
}

gear::Texture * TextureImporter::importTexture2D(const std::string& file) {
    // 加载测试纹理
    int width, height, channels;
    unsigned char* pixels = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    // 做预乘处理
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            unsigned bytePerPixel = channels;
            unsigned char* pixelOffset = pixels + (i + width * j) * bytePerPixel;
            float alpha = pixelOffset[3] / 255.0f;
            pixelOffset[0] *= alpha;
            pixelOffset[1] *= alpha;
            pixelOffset[2] *= alpha;
        }
    }

    uint32_t size = width * height * channels;
    gear::Texture::Builder build;
    build.width(width);
    build.height(height);
    build.format(Blast::FORMAT_R8G8B8A8_UNORM);
    gear::Texture* texture = build.build();
    texture->setData(0, pixels, size);
    stbi_image_free(pixels);
    return texture;
}