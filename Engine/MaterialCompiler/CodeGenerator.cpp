#include "CodeGenerator.h"
#include "ShaderCode.h"

namespace gear {
    std::stringstream& CodeGenerator::generateEpilog(std::stringstream& out) const {
        out << "\n";
        return out;
    }

    std::stringstream& CodeGenerator::generateDefine(std::stringstream& out, const char* name) const {
        out << "#define " << name << "\n";
        return out;
    }

    std::stringstream& CodeGenerator::generateDefine(std::stringstream& out, const char* name, const char* string) const {
        out << "#define " << name << " " << string << "\n";
        return out;
    }

    std::stringstream& CodeGenerator::generateShaderAttributes(std::stringstream& out, Blast::ShaderStage stage, Blast::ShaderSemantic attributes) const {
        if (attributes & Blast::SEMANTIC_NORMAL) {
            generateDefine(out, "HAS_ATTRIBUTE_NORMAL");
        }
        if (attributes & Blast::SEMANTIC_COLOR) {
            generateDefine(out, "HAS_ATTRIBUTE_COLOR");
        }
        if (attributes & Blast::SEMANTIC_TEXCOORD0) {
            generateDefine(out, "HAS_ATTRIBUTE_UV0");
        }
        if (attributes & Blast::SEMANTIC_TEXCOORD1) {
            generateDefine(out, "HAS_ATTRIBUTE_UV1");
        }
        if (attributes & Blast::SEMANTIC_JOINTS) {
            generateDefine(out, "HAS_ATTRIBUTE_BONE_INDICES");
        }
        if (attributes & Blast::SEMANTIC_CUSTOM0) {
            generateDefine(out, "HAS_ATTRIBUTE_CUSTOM0");
        }
        if (attributes & Blast::SEMANTIC_CUSTOM1) {
            generateDefine(out, "HAS_ATTRIBUTE_CUSTOM1");
        }
        if (attributes & Blast::SEMANTIC_CUSTOM2) {
            generateDefine(out, "HAS_ATTRIBUTE_CUSTOM2");
        }
        if (attributes & Blast::SEMANTIC_CUSTOM3) {
            generateDefine(out, "HAS_ATTRIBUTE_CUSTOM3");
        }
        if (attributes & Blast::SEMANTIC_CUSTOM4) {
            generateDefine(out, "HAS_ATTRIBUTE_CUSTOM4");
        }
        if (attributes & Blast::SEMANTIC_CUSTOM5) {
            generateDefine(out, "HAS_ATTRIBUTE_CUSTOM5");
        }

        if (stage == Blast::SHADER_STAGE_VERT) {
            // 声明location
            generateDefine(out, "LOCATION_POSITION", "0");
            generateDefine(out, "LOCATION_NORMAL", "1");
            generateDefine(out, "LOCATION_TANGENT", "2");
            generateDefine(out, "LOCATION_BITANGENT", "3");
            generateDefine(out, "LOCATION_COLOR", "4");
            generateDefine(out, "LOCATION_UV0", "5");
            generateDefine(out, "LOCATION_UV1", "6");
            generateDefine(out, "LOCATION_BONE_INDICES", "7");
            generateDefine(out, "LOCATION_BONE_WEIGHTS", "8");
            generateDefine(out, "HAS_ATTRIBUTE_CUSTOM0", "9");
            generateDefine(out, "HAS_ATTRIBUTE_CUSTOM1", "10");
            generateDefine(out, "HAS_ATTRIBUTE_CUSTOM2", "11");
            generateDefine(out, "HAS_ATTRIBUTE_CUSTOM3", "12");
            generateDefine(out, "HAS_ATTRIBUTE_CUSTOM4", "13");
            generateDefine(out, "HAS_ATTRIBUTE_CUSTOM5", "14");
        } else if (stage == Blast::SHADER_STAGE_FRAG) {
        }
        return out;
    }

    std::stringstream& CodeGenerator::generateShaderInput(std::stringstream& out, Blast::ShaderStage stage) const {
        if (stage == Blast::SHADER_STAGE_VERT) {
            out << INPUTS_VS_DATA << "\n";
        } else if (stage == Blast::SHADER_STAGE_FRAG) {
            out << INPUTS_FS_DATA << "\n";
        }
        return out;
    }

    std::stringstream& CodeGenerator::generateCommonMaterial(std::stringstream& out, Blast::ShaderStage stage) const {
        if (stage == Blast::SHADER_STAGE_VERT) {
            out << MATERIAL_INPUTS_VS_DATA << "\n";
        } else if (stage == Blast::SHADER_STAGE_FRAG) {
            out << MATERIAL_INPUTS_FS_DATA << "\n";
        }
        return out;
    }

    std::stringstream& CodeGenerator::generateShaderMain(std::stringstream& out, Blast::ShaderStage stage) const {
        if (stage == Blast::SHADER_STAGE_VERT) {
            out << MAIN_VS_DATA << "\n";
        } else if (stage == Blast::SHADER_STAGE_FRAG) {
            out << MAIN_FS_DATA << "\n";
        }
        return out;
    }

    std::stringstream& CodeGenerator::generateUniforms(std::stringstream& out, Blast::ShaderStage, const std::vector<std::pair<std::string, std::string>>& params) const {

    }

    std::stringstream& CodeGenerator::generateSamplers(std::stringstream& out, Blast::ShaderStage, const std::vector<std::pair<std::string, std::string>>& params) const {

    }
}
