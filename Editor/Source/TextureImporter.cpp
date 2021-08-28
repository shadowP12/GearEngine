#include "TextureImporter.h"
#include <Resource/Texture.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBIR_FLAG_ALPHA_PREMULTIPLIED
#include <stb_image.h>

gear::Texture* ImportTexture2D(const std::string& file) {
    // 加载纹理数据
    int width, height, channels;
    unsigned char* pixels = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    // 做预乘处理
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            unsigned byte_per_pixel = channels;
            unsigned char* pixel_offset = pixels + (i + width * j) * byte_per_pixel;
            float alpha = pixel_offset[3] / 255.0f;
            pixel_offset[0] *= alpha;
            pixel_offset[1] *= alpha;
            pixel_offset[2] *= alpha;
        }
    }

    uint32_t size = width * height * channels;
    gear::Texture::Builder builder;
    builder.SetWidth(width);
    builder.SetHeight(height);
    builder.SetFormat(blast::FORMAT_R8G8B8A8_UNORM);
    gear::Texture* texture = builder.Build();
    texture->SetData(0, pixels, size);
    stbi_image_free(pixels);
    return texture;
}