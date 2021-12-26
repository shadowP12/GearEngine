#include "CodeGenerator.h"
#include "ShaderCode.h"

namespace gear {
    std::stringstream& CodeGenerator::GenerateEpilog(std::stringstream& out) const {
        out << "\n";
        return out;
    }

    std::stringstream& CodeGenerator::GenerateDefine(std::stringstream& out, const char* name) const {
        out << "#define " << name << "\n";
        return out;
    }

    std::stringstream& CodeGenerator::GenerateDefine(std::stringstream& out, const char* name, const char* string) const {
        out << "#define " << name << " " << string << "\n";
        return out;
    }

    std::stringstream& CodeGenerator::GenerateShaderAttributes(std::stringstream& out, blast::ShaderStage stage, blast::ShaderSemantic attributes) const {
        if (attributes & blast::SEMANTIC_NORMAL) {
            GenerateDefine(out, "HAS_ATTRIBUTE_NORMAL");
        }
        if (attributes & blast::SEMANTIC_COLOR) {
            GenerateDefine(out, "HAS_ATTRIBUTE_COLOR");
        }
        if (attributes & blast::SEMANTIC_TEXCOORD0) {
            GenerateDefine(out, "HAS_ATTRIBUTE_UV0");
        }
        if (attributes & blast::SEMANTIC_TEXCOORD1) {
            GenerateDefine(out, "HAS_ATTRIBUTE_UV1");
        }
        if (attributes & blast::SEMANTIC_JOINTS) {
            GenerateDefine(out, "HAS_ATTRIBUTE_BONE_INDICES");
        }
        if (attributes & blast::SEMANTIC_CUSTOM0) {
            GenerateDefine(out, "HAS_ATTRIBUTE_CUSTOM0");
        }
        if (attributes & blast::SEMANTIC_CUSTOM1) {
            GenerateDefine(out, "HAS_ATTRIBUTE_CUSTOM1");
        }
        if (attributes & blast::SEMANTIC_CUSTOM2) {
            GenerateDefine(out, "HAS_ATTRIBUTE_CUSTOM2");
        }
        if (attributes & blast::SEMANTIC_CUSTOM3) {
            GenerateDefine(out, "HAS_ATTRIBUTE_CUSTOM3");
        }
        if (attributes & blast::SEMANTIC_CUSTOM4) {
            GenerateDefine(out, "HAS_ATTRIBUTE_CUSTOM4");
        }
        if (attributes & blast::SEMANTIC_CUSTOM5) {
            GenerateDefine(out, "HAS_ATTRIBUTE_CUSTOM5");
        }

        if (stage == blast::SHADER_STAGE_VERT) {
            // 声明location
            GenerateDefine(out, "LOCATION_POSITION", "0");
            GenerateDefine(out, "LOCATION_NORMAL", "1");
            GenerateDefine(out, "LOCATION_TANGENT", "2");
            GenerateDefine(out, "LOCATION_BITANGENT", "3");
            GenerateDefine(out, "LOCATION_COLOR", "4");
            GenerateDefine(out, "LOCATION_UV0", "5");
            GenerateDefine(out, "LOCATION_UV1", "6");
            GenerateDefine(out, "LOCATION_BONE_INDICES", "7");
            GenerateDefine(out, "LOCATION_BONE_WEIGHTS", "8");
            GenerateDefine(out, "LOCATION_CUSTOM0", "9");
            GenerateDefine(out, "LOCATION_CUSTOM1", "10");
            GenerateDefine(out, "LOCATION_CUSTOM2", "11");
            GenerateDefine(out, "LOCATION_CUSTOM3", "12");
            GenerateDefine(out, "LOCATION_CUSTOM4", "13");
            GenerateDefine(out, "LOCATION_CUSTOM5", "14");
        } else if (stage == blast::SHADER_STAGE_FRAG) {
        }
        return out;
    }

    std::stringstream& CodeGenerator::GenerateShaderInput(std::stringstream& out, blast::ShaderStage stage) const {
        if (stage == blast::SHADER_STAGE_VERT) {
            out << INPUTS_VS_DATA << "\n";
        } else if (stage == blast::SHADER_STAGE_FRAG) {
            out << INPUTS_FS_DATA << "\n";
        }
        return out;
    }

    std::stringstream& CodeGenerator::GenerateCommonData(std::stringstream& out) const {
        out << COMMON_DATA_FS_DATA << "\n";
        return out;
    }

    std::stringstream& CodeGenerator::GenerateCommonMaterial(std::stringstream& out, blast::ShaderStage stage) const {
        if (stage == blast::SHADER_STAGE_VERT) {
            out << MATERIAL_PARAMS_VS_DATA << "\n";
        } else if (stage == blast::SHADER_STAGE_FRAG) {
            out << MATERIAL_PARAMS_FS_DATA << "\n";
        }
        return out;
    }

    std::stringstream& CodeGenerator::GenerateShaderMain(std::stringstream& out, blast::ShaderStage stage) const {
        if (stage == blast::SHADER_STAGE_VERT) {
            out << MAIN_VS_DATA << "\n";
        } else if (stage == blast::SHADER_STAGE_FRAG) {
            out << MAIN_FS_DATA << "\n";
        }
        return out;
    }

    std::stringstream& CodeGenerator::GenerateShaderDepthMain(std::stringstream& out, blast::ShaderStage stage) const {
        if (stage == blast::SHADER_STAGE_VERT) {
            out << DEPTH_MAIN_VS_DATA << "\n";
        } else if (stage == blast::SHADER_STAGE_FRAG) {
            out << DEPTH_MAIN_FS_DATA << "\n";
        }
        return out;
    }

    std::stringstream& CodeGenerator::GenerateUniforms(std::stringstream& out, const std::unordered_map<std::string, blast::UniformType>& uniforms) const {
        static const std::unordered_map<blast::UniformType, std::string> type_to_str {
                { blast::UNIFORM_BOOL, "bool" },
                { blast::UNIFORM_FLOAT, "float" },
                { blast::UNIFORM_FLOAT2, "vec2" },
                { blast::UNIFORM_FLOAT3, "vec3" },
                { blast::UNIFORM_FLOAT4, "vec4" },
                { blast::UNIFORM_INT, "int" },
                { blast::UNIFORM_INT2, "ivec2" },
                { blast::UNIFORM_INT3, "ivec3" },
                { blast::UNIFORM_INT4, "ivec4" },
                { blast::UNIFORM_MAT4, "mat4" },
        };

        if (uniforms.size() == 0) {
            return out;
        }
        // 从0开始
        out << "layout(std140, set = 0, binding = 0) uniform " << "MaterialUniforms" << " {\n";
        for (auto& uniform : uniforms) {
            out << "    " << type_to_str.at(uniform.second) << " " << uniform.first << ";\n";
        }
        out << "} material_uniforms;\n";
        return out;
    }

    std::stringstream& CodeGenerator::GenerateTextures(std::stringstream& out, const std::unordered_map<std::string, blast::TextureDimension>& textures) const {
        static const std::unordered_map<blast::TextureDimension, std::string> dim_to_str {
                { blast::TEXTURE_DIM_1D, "texture1D" },
                { blast::TEXTURE_DIM_2D, "texture2D" },
                { blast::TEXTURE_DIM_3D, "texture3D" },
                { blast::TEXTURE_DIM_CUBE, "textureCube" }
        };

        if (textures.size() == 0) {
            return out;
        }

        // 纹理槽从1000开始
        uint32_t slot = 1000;
        for (auto& texture : textures) {
            out << "layout(set = 0, binding = " << slot << ") ";
            out << "uniform " << dim_to_str.at(texture.second) << " " << texture.first << ";\n";
            slot++;
        }
        return out;
    }

    std::stringstream& CodeGenerator::GenerateSamplers(std::stringstream& out, const std::vector<std::string>& samplers) const {
        static const std::unordered_map<blast::TextureDimension, std::string> dim_to_str {
                { blast::TEXTURE_DIM_1D, "sampler1D" },
                { blast::TEXTURE_DIM_2D, "sampler2D" },
                { blast::TEXTURE_DIM_3D, "sampler3D" },
                { blast::TEXTURE_DIM_CUBE, "samplerCube" }
        };

        if (samplers.size() == 0) {
            return out;
        }

        // 采样器槽从3000开始
        uint32_t slot = 3000;
        for (auto& sampler : samplers) {
            out << "layout(set = 0, binding = " << slot << ") ";
            out << "uniform " << "sampler" << " " << sampler << ";\n";
            slot++;
        }
        return out;
    }

    std::stringstream& CodeGenerator::GenerateCustomCode(std::stringstream& out, const std::string& code) const {
        out << code << "\n";
        return out;
    }

    std::stringstream& CodeGenerator::GenerateShadingModel(std::stringstream& out, ShadingModel shading_model) const {
        if (shading_model == SHADING_MODEL_UNLIT) {
            out << SHADING_UNLIT_FS_DATA << "\n";
            return out;
        }

        if (shading_model == SHADING_MODEL_LIT) {
            out << LIGHT_DIRECTIONAL_FS_DATA << "\n";
            out << LIGHT_PUNCTUAL_FS_DATA << "\n";
            out << LIGHT_INDIRECT_FS_DATA << "\n";
            out << SHADING_LIT_FS_DATA << "\n";
            return out;
        }
    }
}
