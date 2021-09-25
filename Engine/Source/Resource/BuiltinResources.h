#pragma once
#include <Blast/Gfx/GfxDefine.h>

namespace gear {
    class Material;
    class VertexBuffer;
    class MaterialInstance;
    class BuiltinResources {
    public:
        BuiltinResources();

        ~BuiltinResources();

        void Prepare();

        Material* GetDebugMaterial() { return _debug_ma; }

        Material* GetSkyBoxMaterial() { return _skybox_ma; }

        VertexBuffer* GetQuadBuffer() { return _quad_buffer; }

        VertexBuffer* GetCubeBuffer() { return _cube_buffer; }

    private:
        void CreateDebugMaterial();

        void CreateSkyBoxMaterial();

        void CreateQuadBuffer();

        void CreateCubeBuffer();

    private:
        Material* _debug_ma;
        Material* _skybox_ma;
        VertexBuffer* _quad_buffer;
        VertexBuffer* _cube_buffer;
    };
}
