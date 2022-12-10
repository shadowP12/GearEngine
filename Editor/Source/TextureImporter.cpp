#include "TextureImporter.h"
#include <Resource/Texture.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBIR_FLAG_ALPHA_PREMULTIPLIED
#include <stb_image.h>

std::shared_ptr<blast::GfxTexture> ImportTexture2D(const std::string& file) {
    int width, height, channels;
    unsigned char* pixels = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    // Premultiplied
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            unsigned char* pixel_offset = pixels + (i + width * j) * 4;
            float alpha = pixel_offset[3] / 255.0f;
            pixel_offset[0] *= alpha;
            pixel_offset[1] *= alpha;
            pixel_offset[2] *= alpha;
        }
    }
    auto texture_data = gear::TextureData::Builder()
            .SetWidth(width)
            .SetHeight(height)
            .SetFormat(blast::FORMAT_R8G8B8A8_UNORM)
            .SetData((uint8_t*)pixels, width * height * 4 * sizeof(unsigned char))
            .Build();
    auto texture = texture_data->LoadTexture();
    stbi_image_free(pixels);
    return texture;
}

std::shared_ptr<blast::GfxTexture> ImportTexture2DWithFloat(const std::string& file) {
    int width, height, channels;
    float* pixels = stbi_loadf(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    // Premultiplied
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            float* pixel_offset = pixels + (i + width * j) * 4;
            float alpha = pixel_offset[3];
            pixel_offset[0] *= alpha;
            pixel_offset[1] *= alpha;
            pixel_offset[2] *= alpha;
        }
    }

    auto texture_data = gear::TextureData::Builder()
            .SetWidth(width)
            .SetHeight(height)
            .SetFormat(blast::FORMAT_R32G32B32A32_FLOAT)
            .SetData((uint8_t*)pixels, width * height * 4 * sizeof(float))
            .Build();
    auto texture = texture_data->LoadTexture();
    stbi_image_free(pixels);
    return texture;
}