#include "BuiltinResources.h"
#include "Material.h"
#include "GpuBuffer.h"
#include "GearEngine.h"
#include "MaterialCompiler/MaterialCompiler.h"
namespace gear {
    BuiltinResources::BuiltinResources() {
    }

    BuiltinResources::~BuiltinResources() {
        SAFE_DELETE(_debug_ma);
        SAFE_DELETE(_skybox_ma);
        SAFE_DELETE(_quad_buffer);
        SAFE_DELETE(_cube_buffer);
    }

    void BuiltinResources::Prepare() {
        CreateDebugMaterial();
        CreateSkyBoxMaterial();
        CreateQuadBuffer();
        CreateCubeBuffer();
    }

    void BuiltinResources::CreateDebugMaterial() {
        static const char* g_debug_material_code =
        "{\n"
        "    \"state\": {\n"
        "        \"shading_model\": \"unlit\",\n"
        "        \"blending_mode\": \"opaque\"\n"
        "    },\n"
        "    \"require\": [\n"
        "        \"color\"\n"
        "    ],\n"
        "    \"fragment_code\": \"void ProcessMaterialFragmentParams(inout MaterialFragmentParams params) {\\n    params.base_color = vertex_color;\\n    params.base_color.rgb *= params.base_color.a;\\n }\"\n"
        "}";

        _debug_ma = gEngine.GetMaterialCompiler()->Compile(g_debug_material_code);
    }

    void BuiltinResources::CreateSkyBoxMaterial() {
        static const char* g_skybox_material_code =
                "{\n"
                "    \"state\": {\n"
                "        \"shading_model\": \"unlit\",\n"
                "        \"blending_mode\": \"opaque\"\n"
                "    },\n"
                "    \"require\": [\n"
                "        \"color\"\n"
                "    ],\n"
                "    \"samplers\": [\n"
                "        {\n"
                "            \"name\": \"skybox\",\n"
                "            \"type\": \"sampler_cube\"\n"
                "        }\n"
                "    ],\n"
                "    \"fragment_code\": \"void ProcessMaterialFragmentParams(inout MaterialFragmentParams params) {\\n    params.base_color = vertex_color * texture(skybox, vertex_world_position);\\n    params.base_color.rgb *= params.base_color.a;\\n    params.base_color.rgba = vec4(1.0);\\n}\"\n"
                "}";

        _skybox_ma = gEngine.GetMaterialCompiler()->Compile(g_skybox_material_code);
    }

    void BuiltinResources::CreateQuadBuffer() {
        static float quad_vertices[] = {
                // positions   // texCoords
                -1.0f,  1.0f,  0.0f, 1.0f,
                -1.0f, -1.0f,  0.0f, 0.0f,
                1.0f, -1.0f,  1.0f, 0.0f,

                -1.0f,  1.0f,  0.0f, 1.0f,
                1.0f, -1.0f,  1.0f, 0.0f,
                1.0f,  1.0f,  1.0f, 1.0f
        };

        gear::VertexBuffer::Builder builder;
        builder.SetVertexCount(6);
        builder.SetAttribute(blast::SEMANTIC_POSITION, blast::FORMAT_R32G32_FLOAT);
        builder.SetAttribute(blast::SEMANTIC_TEXCOORD0, blast::FORMAT_R32G32_FLOAT);
        _quad_buffer = builder.Build();
        _quad_buffer->Update(quad_vertices, 0, sizeof(quad_vertices));
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
        builder.SetAttribute(blast::SEMANTIC_POSITION, blast::FORMAT_R32G32B32_FLOAT);
        _cube_buffer = builder.Build();
        _cube_buffer->Update(cube_vertices, 0, sizeof(cube_vertices));
    }

}