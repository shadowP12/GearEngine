#include "BuiltinResources.h"
#include "Material.h"
#include "GpuBuffer.h"
#include "GearEngine.h"
#include "MaterialCompiler/MaterialCompiler.h"
#include "MaterialCompiler/ShaderCode.h"

#include <Blast/Gfx/GfxDevice.h>

namespace gear {
    BuiltinResources::BuiltinResources() {
    }

    BuiltinResources::~BuiltinResources() {
        SAFE_DELETE(blit_ma);
        SAFE_DELETE(debug_ma);
        SAFE_DELETE(skybox_ma);
        SAFE_DELETE(fxaa_ma);
        SAFE_DELETE(quad_buffer);
        SAFE_DELETE(cube_buffer);
        gEngine.GetDevice()->DestroyShader(pano_to_cube_shader);
    }

    void BuiltinResources::Initialize(const std::string& path) {
        blit_ma = gEngine.GetMaterialCompiler()->Compile(path + "Materials/blit.mat", true);
        debug_ma = gEngine.GetMaterialCompiler()->Compile(path + "Materials/debug.mat", true);
        skybox_ma = gEngine.GetMaterialCompiler()->Compile(path + "Materials/skybox.mat", true);
        fxaa_ma = gEngine.GetMaterialCompiler()->Compile(path + "Materials/fxaa.mat", true);
        CreateQuadBuffer();
        CreateCubeBuffer();

        {
            blast::ShaderCompileDesc compile_desc;
            compile_desc.code = PANO_TO_CUBE_COMP_DATA;
            compile_desc.stage = blast::SHADER_STAGE_COMP;
            blast::ShaderCompileResult compile_result = gEngine.GetShaderCompiler()->Compile(compile_desc);
            blast::GfxShaderDesc shader_desc;
            shader_desc.stage = blast::SHADER_STAGE_COMP;
            shader_desc.bytecode = compile_result.bytes.data();
            shader_desc.bytecode_length = compile_result.bytes.size() * sizeof(uint32_t);
            pano_to_cube_shader = gEngine.GetDevice()->CreateShader(shader_desc);
        }
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