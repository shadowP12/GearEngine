#include "MaterialCompiler.h"
#include "CodeGenerator.h"
#include "Utility/FileSystem.h"
#include "Utility/Log.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"

#include <GfxDefine.h>
#include <GfxDevice.h>
#include <GfxShaderCompiler.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

namespace gear {
    MaterialCompiler::MaterialCompiler() {
    }

    MaterialCompiler::~MaterialCompiler() {
    }

    std::shared_ptr<Material> MaterialCompiler::Compile(const std::string& str, bool is_file) {
        blast::GfxDevice* device = gEngine.GetRenderer()->GetDevice();
        blast::GfxShaderCompiler* shader_compiler = gEngine.GetRenderer()->GetShaderCompiler();
        std::string code;
        if (is_file) {
            code = ReadFileData(str);
        } else {
            code = str;
        }

        rapidjson::Document doc;
        if(doc.Parse(code.data()).HasParseError()) {
            LOGE("\n %s \n", code.c_str());
            return nullptr;
        }

        Material::Builder builder;

        // 解析material state
        if (doc.HasMember("state")) {
            if (doc["state"].HasMember("shading_model")) {
                ProcessShadingModel(builder, doc["state"]["shading_model"].GetString());
            }

            if (doc["state"].HasMember("blending_mode")) {
                ProcessBlendingModel(builder, doc["state"]["blending_mode"].GetString());
            }
        }

        // 解析材质所支持的顶点布局
        std::vector<VertexLayoutType> support_vertex_layouts;
        if (doc.HasMember("vertex_layout")) {
            static const std::unordered_map<std::string, VertexLayoutType> str_to_enum {
                    { "p", VLT_P },
                    { "p_t0", VLT_P_T0 },
                    { "debug", VLT_DEBUG },
                    { "ui", VLT_UI },
                    { "static_mesh", VLT_STATIC_MESH },
                    { "skin_mesh", VLT_SKIN_MESH },
            };

            for(uint32_t i = 0; i < doc["vertex_layout"].Size(); i++) {
                if (str_to_enum.find(doc["vertex_layout"][i].GetString()) == str_to_enum.end()) {
                    continue;
                }
                support_vertex_layouts.push_back(str_to_enum.at(doc["vertex_layout"][i].GetString()));
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
                std::string s = doc["uniforms"][i]["name"].GetString();
                std::string t = doc["uniforms"][i]["type"].GetString();
                uniforms[doc["uniforms"][i]["name"].GetString()] = str_to_enum.at(doc["uniforms"][i]["type"].GetString());
            }
        }

        // 解析texture
        std::unordered_map<std::string, blast::TextureDimension> textures;
        if (doc.HasMember("textures")) {
            static const std::unordered_map<std::string, blast::TextureDimension> str_to_enum {
                    { "texture_1d", blast::TEXTURE_DIM_1D },
                    { "texture_2d", blast::TEXTURE_DIM_2D },
                    { "texture_3d", blast::TEXTURE_DIM_3D },
                    { "texture_cube", blast::TEXTURE_DIM_CUBE }
            };
            for(uint32_t i = 0; i < doc["textures"].Size(); i++) {
                textures[doc["textures"][i]["name"].GetString()] = str_to_enum.at(doc["textures"][i]["type"].GetString());
            }
        }

        // 解析sampler
        std::vector<std::string> samplers;
        if (doc.HasMember("samplers")) {
            for(uint32_t i = 0; i < doc["samplers"].Size(); i++) {
                samplers.push_back(doc["samplers"][i].GetString());
            }
        }

        // 解析自定义代码
        std::string vertex_code = "void ProcessMaterialVertexParams(inout MaterialVertexParams params) {}\n";
        std::string fragment_code = "void ProcessMaterialFragmentParams(inout MaterialFragmentParams params) {}\n";
        if (is_file) {
            std::string dir = filesystem::path(str).parent_path().str();
            if (doc.HasMember("vertex_file")) {
                vertex_code = ReadFileData(dir + "/" + doc["vertex_file"].GetString());
            }

            if (doc.HasMember("fragment_file")) {
                fragment_code = ReadFileData(dir + "/" + doc["fragment_file"].GetString());
            }

        } else {
            if (doc.HasMember("vertex_code")) {
                vertex_code = doc["vertex_code"].GetString();
            }

            if (doc.HasMember("fragment_code")) {
                fragment_code = doc["fragment_code"].GetString();
            }
        }

        for (auto& uniform : uniforms) {
            builder.AddUniform(uniform.first, uniform.second);
        }

        for (auto& texture : textures) {
            builder.AddTexture(texture.first, texture.second);
        }

        for (auto& sampler :samplers) {
            builder.AddSampler(sampler);
        }

        // 生成所有可用的变体
        for (auto vertex_layout : support_vertex_layouts) {
            blast::ShaderSemantic attributes;
            if (vertex_layout == VLT_P) {
                attributes = blast::ShaderSemantic::SEMANTIC_POSITION;
            } else if (vertex_layout == VLT_P_T0) {
                attributes = blast::ShaderSemantic::SEMANTIC_POSITION;
                attributes |= blast::ShaderSemantic::SEMANTIC_TEXCOORD0;
            } else if (vertex_layout == VLT_UI) {
                attributes = blast::ShaderSemantic::SEMANTIC_POSITION;
                attributes |= blast::ShaderSemantic::SEMANTIC_TEXCOORD0;
                attributes |= blast::ShaderSemantic::SEMANTIC_COLOR;
            } else if (vertex_layout == VLT_DEBUG) {
                attributes = blast::ShaderSemantic::SEMANTIC_POSITION;
                attributes |= blast::ShaderSemantic::SEMANTIC_COLOR;
            } else if (vertex_layout == VLT_STATIC_MESH) {
                attributes = blast::ShaderSemantic::SEMANTIC_POSITION;
                attributes |= blast::ShaderSemantic::SEMANTIC_TEXCOORD0;
                attributes |= blast::ShaderSemantic::SEMANTIC_NORMAL;
                attributes |= blast::ShaderSemantic::SEMANTIC_TANGENT;
                attributes |= blast::ShaderSemantic::SEMANTIC_BITANGENT;
            } else if (vertex_layout == VLT_SKIN_MESH) {
                attributes = blast::ShaderSemantic::SEMANTIC_POSITION;
                attributes |= blast::ShaderSemantic::SEMANTIC_TEXCOORD0;
                attributes |= blast::ShaderSemantic::SEMANTIC_NORMAL;
                attributes |= blast::ShaderSemantic::SEMANTIC_TANGENT;
                attributes |= blast::ShaderSemantic::SEMANTIC_BITANGENT;
                attributes |= blast::ShaderSemantic::SEMANTIC_JOINTS;
                attributes |= blast::ShaderSemantic::SEMANTIC_WEIGHTS;
            }

            // 需要过滤掉不用使用到的材质变体
            for (MaterialVariant::Key key = 0; key < MATERIAL_VARIANT_COUNT; key++) {
                MaterialVariant variant(key);
                // global只存在一个变体
                if (builder.shading_model == SHADING_MODEL_GLOBAL && key > 0) {
                    continue;
                }

                // unlit着色模型可以减少很多不必要的材质变体
                if (builder.shading_model == SHADING_MODEL_UNLIT) {
                    if (variant.HasDirectionalLighting() || variant.HasDynamicLighting() ||
                        variant.HasIBL() || variant.HasAtmosphere()) {
                        continue;
                    }
                }

                // depth
                if (variant.HasDepth() && !variant.IsValidDepthVariant()) {
                    continue;
                }

                // vertex layout
                if (vertex_layout == VLT_P || vertex_layout == VLT_P_T0 || vertex_layout == VLT_DEBUG || vertex_layout == VLT_UI) {
                    if (variant.HasDirectionalLighting() || variant.HasDynamicLighting() || variant.HasSkinningOrMorphing()) {
                        continue;
                    }
                } else if (vertex_layout == VLT_STATIC_MESH) {
                    if (variant.HasSkinningOrMorphing()) {
                        continue;
                    }
                }

                // 生成vertex shader
                if (variant.FilterVariantVertex(key) == key) {
                    std::stringstream vs;
                    CodeGenerator cg;

                    // 设置define
                    cg.GenerateDefine(vs, "SHADOW_CASCADE_COUNT", std::to_string(SHADOW_CASCADE_COUNT).c_str());

                    if (variant.HasSkinningOrMorphing()) {
                        cg.GenerateDefine(vs, "HAS_SKINNING_OR_MORPHING");
                    }

                    cg.GenerateShaderAttributes(vs, blast::SHADER_STAGE_VERT, attributes);
                    cg.GenerateShaderInput(vs, blast::SHADER_STAGE_VERT);

                    cg.GenerateCommonData(vs);
                    cg.GenerateUniforms(vs, uniforms);
                    cg.GenerateTextures(vs, textures);
                    cg.GenerateSamplers(vs, samplers);

                    cg.GenerateCommonMaterial(vs, blast::SHADER_STAGE_VERT);
                    cg.GenerateCustomCode(vs, vertex_code);

                    if (variant.HasDepth()) {
                        cg.GenerateShaderDepthMain(vs, blast::SHADER_STAGE_VERT);
                    } else {
                        cg.GenerateShaderMain(vs, blast::SHADER_STAGE_VERT);
                    }

                    cg.GenerateEpilog(vs);

                    builder.AddVertShaderCode(key, vertex_layout, vs.str());
                }

                // 生成fragment shader
                if (variant.FilterVariantFragment(key) == key) {
                    std::stringstream fs;
                    CodeGenerator cg;

                    // 设置define
                    cg.GenerateDefine(fs, "SHADOW_CASCADE_COUNT", std::to_string(SHADOW_CASCADE_COUNT).c_str());

                    if (variant.HasDirectionalLighting()) {
                        cg.GenerateDefine(fs, "HAS_DIRECTIONAL_LIGHTING");
                    }

                    if (variant.HasDynamicLighting()) {
                        cg.GenerateDefine(fs, "HAS_DYNAMIC_LIGHTING");
                    }

                    if (variant.HasShadowReceiver()) {
                        cg.GenerateDefine(fs, "HAS_SHADOWING");
                    }

                    if (variant.HasIBL()) {
                        cg.GenerateDefine(fs, "HAS_IBL");
                    }

                    if (variant.HasAtmosphere()) {
                        cg.GenerateDefine(fs, "HAS_ATMOSPHERE");
                    }

                    cg.GenerateShaderAttributes(fs, blast::SHADER_STAGE_FRAG, attributes);
                    cg.GenerateShaderInput(fs, blast::SHADER_STAGE_FRAG);

                    cg.GenerateCommonData(fs);
                    cg.GenerateUniforms(fs, uniforms);
                    cg.GenerateTextures(fs, textures);
                    cg.GenerateSamplers(fs, samplers);

                    cg.GenerateCommonMaterial(fs, blast::SHADER_STAGE_FRAG);
                    cg.GenerateCustomCode(fs, fragment_code);

                    cg.GenerateShadingModel(fs, builder.shading_model);

                    if (variant.HasDepth()) {
                        cg.GenerateShaderDepthMain(fs, blast::SHADER_STAGE_FRAG);
                    } else {
                        cg.GenerateShaderMain(fs, blast::SHADER_STAGE_FRAG);
                    }

                    cg.GenerateEpilog(fs);

                    builder.AddFragShaderCode(key, vertex_layout, fs.str());
                }
            }
        }
        return builder.Build();
    }

    void MaterialCompiler::ProcessShadingModel(Material::Builder& builder, const std::string& value) {
        static const std::unordered_map<std::string, ShadingModel> str_to_enum {
            { "lit", ShadingModel::SHADING_MODEL_LIT },
            { "unlit", ShadingModel::SHADING_MODEL_UNLIT },
            { "global", ShadingModel::SHADING_MODEL_GLOBAL },
        };
        if (str_to_enum.find(value) == str_to_enum.end()) {
            return;
        }
        builder.SetShadingModel(str_to_enum.at(value));
    }

    void MaterialCompiler::ProcessBlendingModel(Material::Builder& builder, const std::string& value) {
        static const std::unordered_map<std::string, BlendStateType> str_to_enum {
                { "opaque", BST_OPAQUE },
                { "transparent", BST_TRANSPARENT }
        };
        if (str_to_enum.find(value) == str_to_enum.end()) {
            return;
        }
        builder.SetBlendState(str_to_enum.at(value));
    }
}