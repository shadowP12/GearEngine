#include "MaterialCompiler.h"
#include "ChunkLexer.h"
#include "ChunkParser.h"
#include "CommonLexer.h"
#include "ParamParser.h"
#include "CodeGenerator.h"
#include "Utility/FileSystem.h"
#include "Utility/Log.h"
#include "Renderer/Renderer.h"
#include "GearEngine.h"
#include <Blast/Gfx/GfxContext.h>
#include <Blast/Utility/ShaderCompiler.h>
#include <Blast/Utility/VulkanShaderCompiler.h>
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
        mShaderCompiler = new Blast::VulkanShaderCompiler();
    }

    MaterialCompiler::~MaterialCompiler() {
        SAFE_DELETE(mShaderCompiler);
    }

    Material* MaterialCompiler::compile(const std::string& path) {
        Blast::GfxContext* context = gEngine.getRenderer()->getContext();
        // 解析材质文件内容
        Material* material = new Material();
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
        // 处理自定义shader代码块
        // TODO: 处理缩进
        std::string materialVertexCode;
        if (chunks.find("vertex") != chunks.end()) {
            materialVertexCode = chunks["vertex"];
        } else {
            materialVertexCode = "void materialVertex(inout MaterialVertexInputs material) {}\n";
        }

        std::string materialFragmentCode;
        if (chunks.find("fragment") != chunks.end()) {
            materialFragmentCode = chunks["fragment"];
        } else {
            materialFragmentCode = "void materialFragment(inout MaterialFragmentInputs material) {}\n";
        }

        // TODO: 后续补充管线状态解析

        // 材质参数只需要初始化一次
        bool initMaterialParams = false;
        // 生成所有可用的变体
        const auto variants = getSurfaceVariants();
        for (const auto& v : variants) {
            // 设置顶点属性布局
            MaterialVariant variant(v.variant);
            Blast::ShaderSemantic attributes = Blast::ShaderSemantic::SEMANTIC_POSITION;
            attributes |= Blast::ShaderSemantic::SEMANTIC_NORMAL;
            attributes |= Blast::ShaderSemantic::SEMANTIC_TANGENT;
            attributes |= Blast::ShaderSemantic::SEMANTIC_BITANGENT;
            attributes |= Blast::ShaderSemantic::SEMANTIC_COLOR;
            attributes |= Blast::ShaderSemantic::SEMANTIC_TEXCOORD0;

            if (v.stage == Blast::ShaderStage::SHADER_STAGE_VERT) {
                std::stringstream vs;
                CodeGenerator cg;
                if (variant.hasSkinningOrMorphing()) {
                    attributes |= Blast::ShaderSemantic::SEMANTIC_JOINTS;
                    attributes |= Blast::ShaderSemantic::SEMANTIC_WEIGHTS;
                    cg.generateDefine(vs, "HAS_SKINNING_OR_MORPHING");
                }
                cg.generateShaderAttributes(vs, v.stage, attributes);
                cg.generateShaderInput(vs, v.stage);
                cg.generateCommonData(vs);
                cg.generateUniforms(vs, v.stage, buildInfo.uniforms);
                cg.generateSamplers(vs, v.stage, buildInfo.samplers);
                cg.generateCommonMaterial(vs, v.stage);
                vs << materialVertexCode << "\n";
                cg.generateShaderMain(vs, v.stage);
                cg.generateEpilog(vs);

                Blast::ShaderCompileDesc compileDesc;
                compileDesc.code = vs.str();
                compileDesc.stage = Blast::SHADER_STAGE_VERT;
                Blast::ShaderCompileResult compileResult = mShaderCompiler->compile(compileDesc);

                Blast::GfxShaderDesc shaderDesc;
                shaderDesc.stage = Blast::SHADER_STAGE_VERT;
                shaderDesc.bytes = compileResult.bytes;
                Blast::GfxShader* vertShader = context->createShader(shaderDesc);
                material->mVertShaderCache[v.variant] = vertShader;

                if (!initMaterialParams) {
                    initMaterialParams = true;
                    int resIdx = -1;
                    for (int i = 0; i < compileResult.resources.size(); ++i) {
                        if (compileResult.resources[i].set == 0) {
                            material->mResources.push_back(compileResult.resources[i]);
                            resIdx = i;
                        } else if (compileResult.resources[i].set == 1 && compileResult.resources[i].reg >= 4) {
                            material->mResources.push_back(compileResult.resources[i]);
                        }
                    }

                    if (resIdx != -1) {
                        for (int i = 0; i < compileResult.variables.size(); i++) {
                            if (compileResult.variables[i].parentIndex == resIdx) {
                                material->mVariables.push_back(compileResult.variables[i]);
                            }
                        }
                    }

                }
            }
            if (v.stage == Blast::ShaderStage::SHADER_STAGE_FRAG) {
                std::stringstream fs;
                CodeGenerator cg;
                cg.generateShaderAttributes(fs, v.stage, attributes);
                cg.generateShaderInput(fs, v.stage);
                cg.generateCommonData(fs);
                cg.generateUniforms(fs, v.stage, buildInfo.uniforms);
                cg.generateSamplers(fs, v.stage, buildInfo.samplers);
                cg.generateCommonMaterial(fs, v.stage);
                fs << materialFragmentCode << "\n";
                cg.generateShaderMain(fs, v.stage);
                cg.generateEpilog(fs);

                Blast::ShaderCompileDesc compileDesc;
                compileDesc.code = fs.str();
                compileDesc.stage = Blast::SHADER_STAGE_FRAG;
                Blast::ShaderCompileResult compileResult = mShaderCompiler->compile(compileDesc);

                Blast::GfxShaderDesc shaderDesc;
                shaderDesc.stage = Blast::SHADER_STAGE_FRAG;
                shaderDesc.bytes = compileResult.bytes;
                Blast::GfxShader* fragShader = context->createShader(shaderDesc);
                material->mFragShaderCache[v.variant] = fragShader;
            }
        }

        return material;
    }
}