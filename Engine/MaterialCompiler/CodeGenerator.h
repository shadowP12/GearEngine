#pragma once
#include <Blast/Gfx/GfxDefine.h>
#include <iosfwd>
#include <string>
#include <sstream>
#include <vector>

namespace gear {
    class CodeGenerator {
    public:
        CodeGenerator() = default;
        std::stringstream& CodeGenerator::generateEpilog(std::stringstream& out) const;
        std::stringstream& generateDefine(std::stringstream& out, const char* name) const;
        std::stringstream& generateDefine(std::stringstream& out, const char* name, const char* string) const;
        std::stringstream& generateShaderAttributes(std::stringstream& out, Blast::ShaderStage stage, Blast::ShaderSemantic attributes) const;
        std::stringstream& generateShaderInput(std::stringstream& out, Blast::ShaderStage stage) const;
        std::stringstream& generateCommonData(std::stringstream& out) const;
        std::stringstream& generateCommonMaterial(std::stringstream& out, Blast::ShaderStage stage) const;
        std::stringstream& generateShaderMain(std::stringstream& out, Blast::ShaderStage stage) const;
        std::stringstream& generateUniforms(std::stringstream& out, Blast::ShaderStage, const std::vector<std::pair<std::string, std::string>>& params) const;
        std::stringstream& generateSamplers(std::stringstream& out, Blast::ShaderStage, const std::vector<std::pair<std::string, std::string>>& params) const;
    };
}