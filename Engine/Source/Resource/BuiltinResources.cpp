#include "BuiltinResources.h"
#include "Material.h"
#include "GearEngine.h"
#include "MaterialCompiler/MaterialCompiler.h"
namespace gear {
    BuiltinResources::BuiltinResources() {
    }

    BuiltinResources::~BuiltinResources() {
        SAFE_DELETE(_debug_ma);
        SAFE_DELETE(_debug_mi);
    }

    void BuiltinResources::Prepare() {
        CreateDebugMaterial();
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
        _debug_mi = _debug_ma->CreateInstance();
    }
}