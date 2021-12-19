#pragma once
#include <Blast/Gfx/GfxDefine.h>

#include <string>

namespace gear {
    class Material;
    class VertexBuffer;
    class MaterialInstance;
    class BuiltinResources {
    public:
        BuiltinResources();

        ~BuiltinResources();

        void Initialize(const std::string&);

        Material* GetBlitMaterial() { return blit_ma; }

        Material* GetDebugMaterial() { return debug_ma; }

        Material* GetSkyBoxMaterial() { return skybox_ma; }

        VertexBuffer* GetQuadBuffer() { return quad_buffer; }

        VertexBuffer* GetCubeBuffer() { return cube_buffer; }

        blast::GfxShader* GetPanoToCubeShader() { return pano_to_cube_shader; }

    private:
        void CreateQuadBuffer();

        void CreateCubeBuffer();

    private:
        Material* blit_ma = nullptr;
        Material* debug_ma = nullptr;
        Material* skybox_ma = nullptr;
        VertexBuffer* quad_buffer = nullptr;
        VertexBuffer* cube_buffer = nullptr;
        blast::GfxShader* pano_to_cube_shader = nullptr;
    };
}
