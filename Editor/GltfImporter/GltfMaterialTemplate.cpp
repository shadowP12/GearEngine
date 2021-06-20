#include "GltfMaterialTemplate.h"
#include "GltfImporter.h"
#include <Engine/Resource/Material.h>
#include <Engine/GearEngine.h>
#include <Engine/MaterialCompiler/MaterialCompiler.h>

static char* gltfMaterialCode =
    "states {\n"
    "shadingModel : lit,\n"
    "blendingMode : ${BLENDING_MODE}\n"
    "}\n"
    "requires {\n"
    "    uv0\n"
    "}\n"
    "${UNIFORMS}\n"
    "${SAMPLERS}\n"
    "fragment {\n"
    "void materialFragment(inout MaterialFragmentInputs material) {\n"
    "${FRAG_SHADER}\n"
    "}\n"
    "}\n"
    "}\n";

GltfMaterialTemplate::GltfMaterialTemplate() {

}

GltfMaterialTemplate::~GltfMaterialTemplate() {

}

bool GltfMaterialTemplate::replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

gear::Material* GltfMaterialTemplate::gen(GltfMaterialConfig* config) {
    std::string code = gltfMaterialCode;

    // 设置混合模式
    if (config->blendingMode == gear::BlendingMode::BLENDING_MODE_TRANSPARENT) {
        replace(code, "${BLENDING_MODE}", "transparent");
    } else if (config->blendingMode == gear::BlendingMode::BLENDING_MODE_MASKED) {
        replace(code, "${BLENDING_MODE}", "masked");
    } else {
        replace(code, "${BLENDING_MODE}", "opaque");
    }

    // 设置uniform变量
    std::string uniforms_code = "";
    uniforms_code += "uniforms {\n";
    uniforms_code += "vec4 base_color;\n";
    uniforms_code += "vec4 metallic_roughness;\n";
    uniforms_code += "}\n";
    replace(code, "${UNIFORMS}", uniforms_code);

    // 设置samplers变量
    std::string samplers_code = "";
    if (config->hasBaseColorTex || config->hasNormalTex || config->hasmetallicRoughnessTex) {
        samplers_code += "samplers {\n";
    }
    if (config->hasBaseColorTex) {
        samplers_code += "sampler2D base_color_texture,\n";
    }

    if (config->hasNormalTex) {
        samplers_code += "sampler2D normal_texture,\n";
    }

    if (config->hasmetallicRoughnessTex) {
        samplers_code += "sampler2D metallic_roughness_texture,\n";
    }
    if (config->hasBaseColorTex || config->hasNormalTex || config->hasmetallicRoughnessTex) {
        samplers_code += "}\n";
    }
    replace(code, "${SAMPLERS}", samplers_code);

    // 设置shader代码
    std::string frag_shader_code = "";
    if (config->hasBaseColorTex) {
        frag_shader_code += "material.base_color = texture(base_color_texture, vertex_uv01.xy);\n";
    } else {
        frag_shader_code += "material.base_color = material_uniforms.base_color;\n";
        frag_shader_code += "material.base_color.rgb *= material.base_color.a\n";
    }

    if (config->hasNormalTex) {
        // TODO
    }

    if (config->hasmetallicRoughnessTex) {
        // TODO
    }
    replace(code, "${FRAG_SHADER}", frag_shader_code);

    gear::Material* material = gear::gEngine.getMaterialCompiler()->compile(code);
    return material;
}
