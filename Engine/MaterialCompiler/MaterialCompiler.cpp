#include "MaterialCompiler.h"
#include "ChunkLexer.h"
#include "ChunkParser.h"
#include "CommonLexer.h"
#include "ParamParser.h"
#include "CodeGenerator.h"
#include "Utility/FileSystem.h"
namespace gear {
    static std::vector<MaterialVariantInfo> getSurfaceVariants() {
        std::vector<MaterialVariantInfo> variants;
        for (uint8_t k = 0; k < MATERIAL_VARIANT_COUNT; k++) {
            // TODO: 后续新增变体时,需要修改此处逻辑
            if (MaterialVariant::filterVariantVertex(k) == k) {
                variants.emplace_back(k, Blast::ShaderStage::SHADER_STAGE_VERT);
            }

            if (MaterialVariant::filterVariantFragment(k) == k) {
                variants.emplace_back(k, Blast::ShaderStage::SHADER_STAGE_FRAG);
            }
        }
        return variants;
    }


    MaterialCompiler::MaterialCompiler() {
    }

    MaterialCompiler::~MaterialCompiler() {
    }

    Material* MaterialCompiler::compile(const std::string& path) {
        // 解析材质文件内容
        MaterialBuildInfo buildInfo;
        std::string materialCode = readFileData(path);
        gear::ChunkLexer chunkLexer;
        chunkLexer.lex(materialCode.c_str(), materialCode.size());
        gear::ChunkParser chunkParser(chunkLexer.getLexemes());
        std::unordered_map<std::string, std::string> chunks = chunkParser.parse();

        if (chunks.find("uniforms") != chunks.end()) {
            gear::CommonLexer uniformLexer;
            uniformLexer.lex(chunks["uniforms"].c_str(), chunks["uniforms"].size());
            gear::ParamParser uniformParamParser(uniformLexer.getLexemes());
            std::vector<gear::ParamParseValue> uniformParams = uniformParamParser.parse();
            for (int i = 0; i < uniformParams.size(); ++i) {
                buildInfo.uniforms.push_back(std::make_pair(uniformParams[i].type, uniformParams[i].name));
            }
        }

        if (chunks.find("samplers") != chunks.end()) {
            gear::CommonLexer samplerLexer;
            samplerLexer.lex(chunks["samplers"].c_str(), chunks["samplers"].size());
            gear::ParamParser samplerParamParser(samplerLexer.getLexemes());
            std::vector<gear::ParamParseValue> samplerParams = samplerParamParser.parse();
            for (int i = 0; i < samplerParams.size(); ++i) {
                buildInfo.samplers.push_back(std::make_pair(samplerParams[i].type, samplerParams[i].name));
            }
        }
        // TODO: 后续补充管线状态解析

        // 生成所有可用的变体
        const auto variants = getSurfaceVariants();
        for (const auto& v : variants) {
            if (v.stage == Blast::ShaderStage::SHADER_STAGE_VERT) {
                std::stringstream vs;
                CodeGenerator cg;
            }
            if (v.stage == Blast::ShaderStage::SHADER_STAGE_FRAG) {
                std::stringstream fs;
                CodeGenerator cg;
            }
        }

        return nullptr;
    }
}