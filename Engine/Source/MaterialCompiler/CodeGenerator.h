#pragma once
#include "Renderer/RenderData.h"
#include <Blast/Gfx/GfxDefine.h>
#include <iosfwd>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace gear {
    class CodeGenerator {
    public:
        CodeGenerator() = default;

        std::stringstream& CodeGenerator::GenerateEpilog(std::stringstream& out) const;

        std::stringstream& GenerateDefine(std::stringstream& out, const char* name) const;

        std::stringstream& GenerateDefine(std::stringstream& out, const char* name, const char* string) const;

        std::stringstream& GenerateUniforms(std::stringstream& out, const std::unordered_map<std::string, blast::UniformType>& uniforms) const;

        std::stringstream& GenerateTextures(std::stringstream& out, const std::unordered_map<std::string, blast::TextureDimension>& textures) const;

        std::stringstream& GenerateSamplers(std::stringstream& out, const std::vector<std::string>& samplers) const;

        std::stringstream& GenerateCommonData(std::stringstream& out) const;

        std::stringstream& GenerateCustomCode(std::stringstream& out, const std::string& code) const;

        std::stringstream& GenerateShaderAttributes(std::stringstream& out, blast::ShaderStage stage, blast::ShaderSemantic attributes) const;

        std::stringstream& GenerateShaderInput(std::stringstream& out, blast::ShaderStage stage) const;

        std::stringstream& GenerateCommonMaterial(std::stringstream& out, blast::ShaderStage stage) const;

        std::stringstream& GenerateShaderMain(std::stringstream& out, blast::ShaderStage stage) const;

        std::stringstream& GenerateShaderDepthMain(std::stringstream& out, blast::ShaderStage stage) const;

        std::stringstream& GenerateShadingModel(std::stringstream& out, ShadingModel shading_model) const;
    };
}