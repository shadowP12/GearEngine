#include "BuiltinResources.h"
#include "Material.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include "Shader.h"
#include "MaterialCompiler/MaterialCompiler.h"
#include "MaterialCompiler/ShaderCode.h"

#include <GfxDevice.h>

namespace gear {
    BuiltinResources::BuiltinResources() {
    }

    BuiltinResources::~BuiltinResources() {
    }

    void BuiltinResources::Initialize(const std::string& path) {
        blast::GfxDevice* device = gEngine.GetRenderer()->GetDevice();
        blast::GfxShaderCompiler* shader_compiler = gEngine.GetRenderer()->GetShaderCompiler();

        blit_ma = gEngine.GetMaterialCompiler()->Compile(path + "/Materials/blit.mat", true);
        debug_ma = gEngine.GetMaterialCompiler()->Compile(path + "/Materials/debug.mat", true);
        skybox_ma = gEngine.GetMaterialCompiler()->Compile(path + "/Materials/skybox.mat", true);
        fxaa_ma = gEngine.GetMaterialCompiler()->Compile(path + "/Materials/fxaa.mat", true);
        CreateQuadBuffer();
        CreateCubeBuffer();

        {
            blast::ShaderCompileDesc compile_desc;
            compile_desc.code = PANO_TO_CUBE_COMP_DATA;
            compile_desc.stage = blast::SHADER_STAGE_COMP;
            blast::ShaderCompileResult compile_result = shader_compiler->Compile(compile_desc);
            blast::GfxShaderDesc shader_desc;
            shader_desc.stage = blast::SHADER_STAGE_COMP;
            shader_desc.bytecode = compile_result.bytes.data();
            shader_desc.bytecode_length = compile_result.bytes.size() * sizeof(uint32_t);
            pano_to_cube_shader = std::shared_ptr<blast::GfxShader>(device->CreateShader(shader_desc));
        }

        {
            blast::ShaderCompileDesc compile_desc;
            compile_desc.code = COMPUTE_IRRADIANCE_MAP_COMP_DATA;
            compile_desc.stage = blast::SHADER_STAGE_COMP;
            blast::ShaderCompileResult compile_result = shader_compiler->Compile(compile_desc);
            blast::GfxShaderDesc shader_desc;
            shader_desc.stage = blast::SHADER_STAGE_COMP;
            shader_desc.bytecode = compile_result.bytes.data();
            shader_desc.bytecode_length = compile_result.bytes.size() * sizeof(uint32_t);
            compute_irradiance_map_shader = std::shared_ptr<blast::GfxShader>(device->CreateShader(shader_desc));
        }

        {
            blast::ShaderCompileDesc compile_desc;
            compile_desc.code = COMPUTE_SPECULAR_MAP_COMP_DATA;
            compile_desc.stage = blast::SHADER_STAGE_COMP;
            blast::ShaderCompileResult compile_result = shader_compiler->Compile(compile_desc);
            blast::GfxShaderDesc shader_desc;
            shader_desc.stage = blast::SHADER_STAGE_COMP;
            shader_desc.bytecode = compile_result.bytes.data();
            shader_desc.bytecode_length = compile_result.bytes.size() * sizeof(uint32_t);
            compute_specular_map_shader = std::shared_ptr<blast::GfxShader>(device->CreateShader(shader_desc));
        }

        {
            blast::ShaderCompileDesc compile_desc;
            compile_desc.code = BRDF_INTEGRATION_COMP_DATA;
            compile_desc.stage = blast::SHADER_STAGE_COMP;
            blast::ShaderCompileResult compile_result = shader_compiler->Compile(compile_desc);
            blast::GfxShaderDesc shader_desc;
            shader_desc.stage = blast::SHADER_STAGE_COMP;
            shader_desc.bytecode = compile_result.bytes.data();
            shader_desc.bytecode_length = compile_result.bytes.size() * sizeof(uint32_t);
            brdf_integration_shader = std::shared_ptr<blast::GfxShader>(device->CreateShader(shader_desc));
        }

        // Atmosphere
        {
            SimpleShaderDesc desc;
            desc.stage = blast::SHADER_STAGE_VERT;
            desc.code = ATMOSPHERE_COMON_VS_DATA;
            atmosphere_comon_vs = std::make_shared<SimpleShader>(desc);

            desc.stage = blast::SHADER_STAGE_FRAG;
            desc.code = ATMOSPHERE_COMPUTE_TRANSMITTANCE_FS_DATA;
            atmosphere_compute_transmittance_fs = std::make_shared<SimpleShader>(desc);

            desc.stage = blast::SHADER_STAGE_COMP;
            desc.code = ATMOSPHERE_COMPUTE_MULTI_SCATTERING_COMP_DATA;
            atmosphere_compute_multi_scatt_cs = std::make_shared<SimpleShader>(desc);

            desc.stage = blast::SHADER_STAGE_FRAG;
            desc.code = ATMOSPHERE_RAY_MARCHING_FS_DATA;
            atmosphere_raymarching_fs = std::make_shared<SimpleShader>(desc);
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

        Renderer* renderer = gEngine.GetRenderer();
        blast::GfxDevice* device = renderer->GetDevice();
        blast::GfxCommandBuffer* cmd = renderer->GetCurrentCommandBuffer();

        blast::GfxBufferDesc buffer_desc{};
        buffer_desc.size = sizeof(quad_vertices);
        buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
        buffer_desc.res_usage = blast::RESOURCE_USAGE_VERTEX_BUFFER;
        quad_buffer = std::shared_ptr<blast::GfxBuffer>(device->CreateBuffer(buffer_desc));
        renderer->UpdateVertexBuffer(cmd, quad_buffer.get(), quad_vertices, sizeof(quad_vertices));
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

        Renderer* renderer = gEngine.GetRenderer();
        blast::GfxDevice* device = renderer->GetDevice();
        blast::GfxCommandBuffer* cmd = renderer->GetCurrentCommandBuffer();

        blast::GfxBufferDesc buffer_desc{};
        buffer_desc.size = sizeof(cube_vertices);
        buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
        buffer_desc.res_usage = blast::RESOURCE_USAGE_VERTEX_BUFFER;
        cube_buffer = std::shared_ptr<blast::GfxBuffer>(device->CreateBuffer(buffer_desc));
        renderer->UpdateVertexBuffer(cmd, cube_buffer.get(), cube_vertices, sizeof(cube_vertices));
    }

    std::shared_ptr<blast::GfxShader> BuiltinResources::GetAtmosphereCommonVS(uint32_t var) {
        return atmosphere_comon_vs->GetShader(var);
    }

    std::shared_ptr<blast::GfxShader> BuiltinResources::GetAtmosphereComputeTransmittanceFS(uint32_t var) {
        return atmosphere_compute_transmittance_fs->GetShader(var);
    }

    std::shared_ptr<blast::GfxShader> BuiltinResources::GetAtmosphereComputeMultiScattCS(uint32_t var) {
        return atmosphere_compute_multi_scatt_cs->GetShader(var);
    }

    std::shared_ptr<blast::GfxShader> BuiltinResources::GetAtmosphereRayMarchingFS(uint32_t var) {
        return atmosphere_raymarching_fs->GetShader(var);
    }
}