#include "MaterialCompiler.h"
#include "CodeGenerator.h"
#include "Utility/FileSystem.h"
#include "Utility/Log.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include <Blast/Gfx/GfxContext.h>
#include <Blast/Utility/ShaderCompiler.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

namespace gear {
    MaterialCompiler::MaterialCompiler() {
    }

    MaterialCompiler::~MaterialCompiler() {
    }

    Material* MaterialCompiler::Compile(const std::string& code) {
        rapidjson::Document doc;
        if(doc.Parse(code.data()).HasParseError()) {
            return nullptr;
        }

        Material::Builder builder;

        // 解析material state
        if (doc.HasMember("state")) {
            if (doc["state"].HasMember("shading_model")) {
                ProcessShadingModel(builder, doc["state"]["shading_model"].GetString());
            }

            if (doc["State"].HasMember("blending_mode")) {
                ProcessShadingModel(builder, doc["state"]["blending_mode"].GetString());
            }
        }

        // 解析材质所需要的顶点属性
        blast::ShaderSemantic semantics = blast::SEMANTIC_UNDEFINED;
        if (doc.HasMember("require")) {
            static const std::unordered_map<std::string, blast::ShaderSemantic> str_to_enum {
                    { "color", blast::SEMANTIC_COLOR },
                    { "position", blast::SEMANTIC_POSITION },
                    { "tangents", blast::SEMANTIC_NORMAL },
                    { "uv0", blast::SEMANTIC_TEXCOORD0 },
                    { "uv1", blast::SEMANTIC_TEXCOORD1 },
                    { "custom0", blast::SEMANTIC_CUSTOM0 },
                    { "custom1", blast::SEMANTIC_CUSTOM1 },
                    { "custom2", blast::SEMANTIC_CUSTOM2 },
                    { "custom3", blast::SEMANTIC_CUSTOM3 },
                    { "custom4", blast::SEMANTIC_CUSTOM4 },
                    { "custom5", blast::SEMANTIC_CUSTOM5 },
            };

            for(uint32_t i = 0; i < doc["require"].Size(); i++) {
                if (str_to_enum.find(doc["require"][i].GetString()) == str_to_enum.end()) {
                    continue;
                }
                semantics |= str_to_enum.at(doc["require"][i].GetString());
            }
        }

        // 解析uniforms
        std::unordered_map<std::string, blast::UniformType> uniforms;
        if (doc.HasMember("uniforms")) {
            static const std::unordered_map<std::string, blast::UniformType> str_to_enum {
                    { "bool", blast::UNIFORM_BOOL },
                    { "float", blast::UNIFORM_FLOAT },
                    { "float2", blast::UNIFORM_FLOAT2 },
                    { "float3", blast::UNIFORM_FLOAT3 },
                    { "float4", blast::UNIFORM_FLOAT4 },
                    { "int", blast::UNIFORM_INT },
                    { "int2", blast::UNIFORM_INT2 },
                    { "int3", blast::UNIFORM_INT3 },
                    { "int4", blast::UNIFORM_INT4 },
                    { "mat4", blast::UNIFORM_MAT4 },
            };
            for(uint32_t i = 0; i < doc["uniforms"].Size(); i++) {
                uniforms[doc["uniforms"][i]["name"].GetString()] = str_to_enum.at(doc["uniforms"][i]["type"].GetString());
            }
        }

        // 解析sampler
        std::unordered_map<std::string, blast::TextureDimension> samplers;
        if (doc.HasMember("samplers")) {
            static const std::unordered_map<std::string, blast::TextureDimension> str_to_enum {
                    { "sampler_1d", blast::TEXTURE_DIM_1D },
                    { "sampler_2d", blast::TEXTURE_DIM_2D },
                    { "sampler_3d", blast::TEXTURE_DIM_3D },
                    { "sampler_cube", blast::TEXTURE_DIM_CUBE }
            };
            for(uint32_t i = 0; i < doc["samplers"].Size(); i++) {
                samplers[doc["samplers"][i]["name"].GetString()] = str_to_enum.at(doc["samplers"][i]["type"].GetString());
            }
        }

        // 解析自定义代码
        std::string vertex_code;
        if (doc.HasMember("vertex_code")) {
            vertex_code = doc["vertex_code"].GetString();
        } else {
            vertex_code = "void MaterialVertex(inout MaterialVertexInputs material) {}\n";
        }

        std::string fragment_code;
        if (doc.HasMember("fragment_code")) {
            fragment_code = doc["fragment_code"].GetString();
        } else {
            fragment_code = "void MaterialFragment(inout MaterialFragmentInputs material) {}\n";
        }

        for (auto& uniform : uniforms) {
            builder.AddUniform(uniform.first, uniform.second);
        }

        for (auto& sampler :samplers) {
            builder.AddSampler(sampler.first, sampler.second);
        }

        // 生成所有可用的变体
        // 需要过滤掉不用使用到的材质变体
        for (MaterialVariant::Key key = 0; key < MATERIAL_VARIANT_COUNT; key++) {
            MaterialVariant variant(key);

            // 设置当前材质变体的顶点属性
            blast::ShaderSemantic attributes = semantics;
            attributes |= blast::ShaderSemantic::SEMANTIC_POSITION;
            if (builder._render_state.shading_model != SHADING_MODEL_UNLIT) {
                attributes |= blast::ShaderSemantic::SEMANTIC_NORMAL;
                attributes |= blast::ShaderSemantic::SEMANTIC_TANGENT;
                attributes |= blast::ShaderSemantic::SEMANTIC_BITANGENT;
            }
            if (attributes & blast::ShaderSemantic::SEMANTIC_NORMAL) {
                attributes |= blast::ShaderSemantic::SEMANTIC_TANGENT;
                attributes |= blast::ShaderSemantic::SEMANTIC_BITANGENT;
            }
            if (variant.HasSkinningOrMorphing()) {
                attributes |= blast::ShaderSemantic::SEMANTIC_JOINTS;
                attributes |= blast::ShaderSemantic::SEMANTIC_WEIGHTS;
            }

            // unlit着色模型不需要任何光照相关的材质变体
            if (builder._render_state.shading_model == SHADING_MODEL_UNLIT) {
                if (variant.HasDirectionalLighting() || variant.HasDynamicLighting()) {
                    continue;
                }
            }

            // 生成vertex shader
            if (variant.FilterVariantVertex(key) == key) {
                std::stringstream vs;
                CodeGenerator cg;

                if (variant.HasSkinningOrMorphing()) {
                    cg.GenerateDefine(vs, "HAS_SKINNING_OR_MORPHING");
                }

                cg.GenerateShaderAttributes(vs, blast::SHADER_STAGE_VERT, attributes);
                cg.GenerateShaderInput(vs, blast::SHADER_STAGE_VERT);

                cg.GenerateCommonData(vs);
                cg.GenerateUniforms(vs, uniforms);
                cg.GenerateSamplers(vs, samplers);

                cg.GenerateCommonMaterial(vs, blast::SHADER_STAGE_VERT);
                cg.GenerateCustomCode(vs, vertex_code);
                cg.GenerateShaderMain(vs, blast::SHADER_STAGE_VERT);
                cg.GenerateEpilog(vs);

                blast::ShaderCompileDesc compile_desc;
                compile_desc.code = vs.str();
                compile_desc.stage = blast::SHADER_STAGE_VERT;
                blast::ShaderCompileResult compile_result = gEngine.GetRenderer()->GetShaderCompiler()->Compile(compile_desc);
                if (compile_result.success) {
                    blast::GfxShaderDesc shader_desc;
                    shader_desc.stage = blast::SHADER_STAGE_VERT;
                    shader_desc.bytes = compile_result.bytes;
                    blast::GfxShader* vert_shader = gEngine.GetRenderer()->GetContext()->CreateShader(shader_desc);
                    builder.AddVertShader(key, vert_shader);
                } else {
                    LOGE("\n %s \n", vs.str().c_str());
                }
            }

            // 生成fragment shader
            if (variant.FilterVariantFragment(key) == key) {
                std::stringstream fs;
                CodeGenerator cg;
                cg.GenerateShaderAttributes(fs, blast::SHADER_STAGE_FRAG, attributes);
                cg.GenerateShaderInput(fs, blast::SHADER_STAGE_FRAG);

                cg.GenerateCommonData(fs);
                cg.GenerateUniforms(fs, uniforms);
                cg.GenerateSamplers(fs, samplers);

                cg.GenerateCommonMaterial(fs, blast::SHADER_STAGE_FRAG);
                cg.GenerateCustomCode(fs, fragment_code);
                cg.GenerateShaderMain(fs, blast::SHADER_STAGE_FRAG);
                cg.GenerateEpilog(fs);

                blast::ShaderCompileDesc compile_desc;
                compile_desc.code = fs.str();
                compile_desc.stage = blast::SHADER_STAGE_FRAG;
                blast::ShaderCompileResult compile_result = gEngine.GetRenderer()->GetShaderCompiler()->Compile(compile_desc);
                if (compile_result.success) {
                    blast::GfxShaderDesc shader_desc;
                    shader_desc.stage = blast::SHADER_STAGE_VERT;
                    shader_desc.bytes = compile_result.bytes;
                    blast::GfxShader* frag_shader = gEngine.GetRenderer()->GetContext()->CreateShader(shader_desc);
                    builder.AddVertShader(key, frag_shader);
                } else {
                    LOGE("\n %s \n", fs.str().c_str());
                }
            }
        }
        return builder.Build();
    }

    void MaterialCompiler::ProcessShadingModel(Material::Builder& builder, const std::string& value) {
        static const std::unordered_map<std::string, ShadingModel> str_to_enum {
            { "lit", ShadingModel::SHADING_MODEL_LIT },
            { "unlit", ShadingModel::SHADING_MODEL_UNLIT },
        };
        if (str_to_enum.find(value) == str_to_enum.end()) {
            return;
        }
        builder.SetShadingModel(str_to_enum.at(value));
    }

    void MaterialCompiler::ProcessBlendingModel(Material::Builder& builder, const std::string& value) {
        static const std::unordered_map<std::string, BlendingMode> str_to_enum {
                { "opaque", BlendingMode::BLENDING_MODE_OPAQUE },
                { "transparent", BlendingMode::BLENDING_MODE_TRANSPARENT },
                { "masked", BlendingMode::BLENDING_MODE_MASKED },
        };
        if (str_to_enum.find(value) == str_to_enum.end()) {
            return;
        }
        builder.SetBlendingMode(str_to_enum.at(value));
    }
}