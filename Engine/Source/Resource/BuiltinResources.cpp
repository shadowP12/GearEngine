#include "BuiltinResources.h"
#include "Material.h"
#include "GpuBuffer.h"
#include "GearEngine.h"
#include "MaterialCompiler/MaterialCompiler.h"

namespace gear {
    BuiltinResources::BuiltinResources() {
    }

    BuiltinResources::~BuiltinResources() {
        SAFE_DELETE(blit_ma);
        SAFE_DELETE(debug_ma);
        SAFE_DELETE(skybox_ma);
        SAFE_DELETE(equirectangular_to_cube_ma);
        SAFE_DELETE(quad_buffer);
        SAFE_DELETE(cube_buffer);
    }

    void BuiltinResources::Initialize(const std::string& path) {
        blit_ma = gEngine.GetMaterialCompiler()->Compile(path + "Materials/blit.mat", true);
        debug_ma = gEngine.GetMaterialCompiler()->Compile(path + "Materials/debug.mat", true);
        skybox_ma = gEngine.GetMaterialCompiler()->Compile(path + "Materials/skybox.mat", true);
        equirectangular_to_cube_ma = gEngine.GetMaterialCompiler()->Compile(path + "Materials/equirectangular_to_cube.mat", true);
        CreateQuadBuffer();
        CreateCubeBuffer();
    }

    void BuiltinResources::CreateQuadBuffer() {
        static float quad_vertices[] = {
                // positions        // texCoords
                -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f
        };

        gear::VertexBuffer::Builder builder;
        builder.SetVertexCount(6);
        builder.SetVertexLayoutType(VLT_P_T0);
        quad_buffer = builder.Build();
        quad_buffer->UpdateData(quad_vertices, sizeof(quad_vertices));
    }

    void BuiltinResources::CreateCubeBuffer() {
        static float cube_vertices[] = {
                // positions
                -1.0f,  1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                -1.0f,  1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f,  1.0f
        };

        gear::VertexBuffer::Builder builder;
        builder.SetVertexCount(36);
        builder.SetVertexLayoutType(VLT_P);
        cube_buffer = builder.Build();
        cube_buffer->UpdateData(cube_vertices, sizeof(cube_vertices));
    }

}