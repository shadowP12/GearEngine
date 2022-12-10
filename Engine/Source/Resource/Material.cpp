#include "Resource/Material.h"
#include "Resource/Texture.h"
#include "JobSystem/JobSystem.h"
#include <Utility/Log.h>
#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include <GfxDefine.h>
#include <GfxDevice.h>
#include <functional>

namespace gear {
    uint32_t Material::global_material_id = 0;

    MaterialVariant::Key MaterialVariant::FilterVariantVertex(Key variant) {
        return variant & VERTEX_MASK;
    }

    MaterialVariant::Key MaterialVariant::FilterVariantFragment(Key variant) {
        return variant & FRAGMENT_MASK;
    }

    MaterialVariant::Key MaterialVariant::FilterVariantShadingMode(Key variant, ShadingModel shading_model) {
        if (shading_model == SHADING_MODEL_UNLIT) {
            Set(variant, false, DYNAMIC_LIGHTING);
            Set(variant, false, DIRECTIONAL_LIGHTING);
        }
        return variant;
    }

    MaterialVariant::Key MaterialVariant::FilterVariantVertexLayout(Key variant, VertexLayoutType vertex_layout_type) {
        if (vertex_layout_type == VLT_P || vertex_layout_type == VLT_P_T0 || vertex_layout_type == VLT_DEBUG || vertex_layout_type == VLT_UI) {
            Set(variant, false, DYNAMIC_LIGHTING);
            Set(variant, false, DIRECTIONAL_LIGHTING);
            Set(variant, false, SKINNING_OR_MORPHING);
        } else if (vertex_layout_type == VLT_STATIC_MESH) {
            Set(variant, false, SKINNING_OR_MORPHING);
        }
        return variant;
    }

    static uint32_t GetUniformTypeBaseAlignment(blast::UniformType type) {
        switch (type) {
            case blast::UNIFORM_BOOL:
            case blast::UNIFORM_FLOAT:
            case blast::UNIFORM_INT:
            case blast::UNIFORM_UINT:
                return 1;
            case blast::UNIFORM_FLOAT2:
            case blast::UNIFORM_INT2:
            case blast::UNIFORM_UINT2:
                return 2;
            case blast::UNIFORM_FLOAT3:
            case blast::UNIFORM_FLOAT4:
            case blast::UNIFORM_INT3:
            case blast::UNIFORM_INT4:
            case blast::UNIFORM_UINT3:
            case blast::UNIFORM_UINT4:
            case blast::UNIFORM_MAT4:
                return 4;
        }
    }

    static uint32_t GetUniformTypeStride(blast::UniformType type) {
        switch (type) {
            case blast::UNIFORM_BOOL:
            case blast::UNIFORM_INT:
            case blast::UNIFORM_UINT:
            case blast::UNIFORM_FLOAT:
                return 1;
            case blast::UNIFORM_INT2:
            case blast::UNIFORM_UINT2:
            case blast::UNIFORM_FLOAT2:
                return 2;
            case blast::UNIFORM_INT3:
            case blast::UNIFORM_UINT3:
            case blast::UNIFORM_FLOAT3:
                return 3;
            case blast::UNIFORM_INT4:
            case blast::UNIFORM_UINT4:
            case blast::UNIFORM_FLOAT4:
                return 4;
            case blast::UNIFORM_MAT4:
                return 16;
        }
    }

    Material::Builder& Material::Builder::SetShadingModel(ShadingModel shading_model) {
        this->shading_model = shading_model;
        return *this;
    }

    Material::Builder& Material::Builder::SetBlendState(BlendStateType blend_state) {
        this->blend_state = blend_state;
        return *this;
    }

    Material::Builder& Material::Builder::SetTopology(blast::PrimitiveTopology topo) {
        this->topo = topo;
        return *this;
    }

    Material::Builder& Material::Builder::AddVertShader(MaterialVariant::Key key, VertexLayoutType vertex_layout_type, blast::GfxShader* shader) {
        std::size_t shader_hash = 0;
        HashCombine(shader_hash, key);
        HashCombine(shader_hash, vertex_layout_type);
        vert_shader_cache[shader_hash] = shader;
        return *this;
    }

    Material::Builder& Material::Builder::AddFragShader(MaterialVariant::Key key, VertexLayoutType vertex_layout_type, blast::GfxShader* shader) {
        std::size_t shader_hash = 0;
        HashCombine(shader_hash, key);
        HashCombine(shader_hash, vertex_layout_type);
        frag_shader_cache[shader_hash] = shader;
        return *this;
    }

    Material::Builder& Material::Builder::AddVertShaderCode(MaterialVariant::Key key, VertexLayoutType vertex_layout_type, const std::string& code) {
        std::size_t shader_hash = 0;
        HashCombine(shader_hash, key);
        HashCombine(shader_hash, vertex_layout_type);
        vert_shader_code_cache[shader_hash] = code;
        return *this;
    }

    Material::Builder& Material::Builder::AddFragShaderCode(MaterialVariant::Key key, VertexLayoutType vertex_layout_type, const std::string& code) {
        std::size_t shader_hash = 0;
        HashCombine(shader_hash, key);
        HashCombine(shader_hash, vertex_layout_type);
        frag_shader_code_cache[shader_hash] = code;
        return *this;
    }

    Material::Builder& Material::Builder::AddUniform(const std::string& name, const blast::UniformType& type) {
        uniforms[name] = type;
        return *this;
    }

    Material::Builder& Material::Builder::AddTexture(const std::string& name, const blast::TextureDimension& dim) {
        textures[name] = dim;
        return *this;
    }

    Material::Builder& Material::Builder::AddSampler(const std::string& name) {
        samplers.push_back(name);
        return *this;
    }

    std::shared_ptr<Material> Material::Builder::Build() {
        return std::shared_ptr<Material>(new Material(this));
    }

    Material::Material(Builder* builder) {
        shading_model = builder->shading_model;
        blend_state = builder->blend_state;
        topo = builder->topo;
        uniforms = builder->uniforms;
        textures = builder->textures;
        samplers = builder->samplers;
        vert_shader_code_cache = builder->vert_shader_code_cache;
        frag_shader_code_cache = builder->frag_shader_code_cache;
        material_id = global_material_id;
        global_material_id++;
    }

    Material::~Material() {
        vert_shader_cache.clear();
        frag_shader_cache.clear();
    }

    blast::GfxShader* Material::GetVertShader(MaterialVariant::Key variant, VertexLayoutType vertex_layout_type) {
        MaterialVariant::Key key = MaterialVariant::FilterVariantVertex(variant);
        key = MaterialVariant::FilterVariantShadingMode(key, shading_model);
        key = MaterialVariant::FilterVariantVertexLayout(key, vertex_layout_type);

        std::size_t shader_hash = 0;
        HashCombine(shader_hash, key);
        HashCombine(shader_hash, vertex_layout_type);

        blast::GfxShader* shader = nullptr;
        vs_cache_mutex.lock();
        auto shader_iter = vert_shader_cache.find(shader_hash);
        if (shader_iter != vert_shader_cache.end()) {
            shader = shader_iter->second.get();
        }
        vs_cache_mutex.unlock();

        if (shader == nullptr) {
            // 判断是否已经触发过编译
            auto triggered_iter = vert_shader_triggered_cache.find(shader_hash);
            if (triggered_iter == vert_shader_triggered_cache.end()) {
                vert_shader_triggered_cache[shader_hash] = true;

                // 若没有触发过编译，则异步编译对应shader
                auto code_iter = vert_shader_code_cache.find(shader_hash);
                if (code_iter != vert_shader_code_cache.end()) {
                    gEngine.GetJobSystem()->ExecuteJob([this, shader_hash, code_iter](JobSystem::JobArgs args) {
                        blast::ShaderCompileDesc compile_desc;
                        compile_desc.code = code_iter->second;
                        compile_desc.stage = blast::SHADER_STAGE_VERT;
                        blast::ShaderCompileResult compile_result = gEngine.GetRenderer()->GetShaderCompiler()->Compile(compile_desc);
                        if (compile_result.success) {
                            blast::GfxShaderDesc shader_desc;
                            shader_desc.stage = blast::SHADER_STAGE_VERT;
                            shader_desc.bytecode = compile_result.bytes.data();
                            shader_desc.bytecode_length = compile_result.bytes.size() * sizeof(uint32_t);
                            blast::GfxShader* vert_shader = gEngine.GetRenderer()->GetDevice()->CreateShader(shader_desc);

                            // Compiled shader into cache
                            this->vs_cache_mutex.lock();
                            this->vert_shader_cache[shader_hash] = std::shared_ptr<blast::GfxShader>(vert_shader);
                            this->vs_cache_mutex.unlock();
                        } else {
                            LOGE("\n %s \n", code_iter->second.c_str());
                        }
                    });
                }
            }
        }

        return shader;
    }

    blast::GfxShader* Material::GetFragShader(MaterialVariant::Key variant, VertexLayoutType vertex_layout_type) {
        MaterialVariant::Key key = MaterialVariant::FilterVariantFragment(variant);
        key = MaterialVariant::FilterVariantShadingMode(key, shading_model);
        key = MaterialVariant::FilterVariantVertexLayout(key, vertex_layout_type);

        std::size_t shader_hash = 0;
        HashCombine(shader_hash, key);
        HashCombine(shader_hash, vertex_layout_type);

        blast::GfxShader* shader = nullptr;
        fs_cache_mutex.lock();
        auto shader_iter = frag_shader_cache.find(shader_hash);
        if (shader_iter != frag_shader_cache.end()) {
            shader = shader_iter->second.get();
        }
        fs_cache_mutex.unlock();

        if (shader == nullptr) {
            // 判断是否已经触发过编译
            auto triggered_iter = frag_shader_triggered_cache.find(shader_hash);
            if (triggered_iter == frag_shader_triggered_cache.end()) {
                frag_shader_triggered_cache[shader_hash] = true;

                // 若没有触发过编译，则异步编译对应shader
                auto code_iter = frag_shader_code_cache.find(shader_hash);
                if (code_iter != frag_shader_code_cache.end()) {
                    gEngine.GetJobSystem()->ExecuteJob([this, shader_hash, code_iter](JobSystem::JobArgs args) {
                        blast::ShaderCompileDesc compile_desc;
                        compile_desc.code = code_iter->second;
                        compile_desc.stage = blast::SHADER_STAGE_FRAG;
                        blast::ShaderCompileResult compile_result = gEngine.GetRenderer()->GetShaderCompiler()->Compile(compile_desc);
                        if (compile_result.success) {
                            blast::GfxShaderDesc shader_desc;
                            shader_desc.stage = blast::SHADER_STAGE_FRAG;
                            shader_desc.bytecode = compile_result.bytes.data();
                            shader_desc.bytecode_length = compile_result.bytes.size() * sizeof(uint32_t);
                            blast::GfxShader* frag_shader = gEngine.GetRenderer()->GetDevice()->CreateShader(shader_desc);

                            // Compiled shader into cache
                            this->fs_cache_mutex.lock();
                            this->frag_shader_cache[shader_hash] = std::shared_ptr<blast::GfxShader>(frag_shader);
                            this->fs_cache_mutex.unlock();
                        } else {
                            LOGE("\n %s \n", code_iter->second.c_str());
                        }
                    });
                }
            }
        }

        return shader;
    }

    std::shared_ptr<MaterialInstance> Material::CreateInstance() {
        MaterialInstance* mi = new MaterialInstance(shared_from_this());
        mi->material_instance_id = current_material_instance_id;
        current_material_instance_id++;
        return std::shared_ptr<MaterialInstance>(mi);
    }

    MaterialInstance::MaterialInstance(std::shared_ptr<Material> material) {
        this->material = material;

        // Uniform
        uint32_t offset = 0;
        for (auto& uniform : material->uniforms) {
            std::tuple<blast::UniformType, uint32_t> variable = {};
            std::get<0>(variable) = uniform.second;

            uint32_t alignment = GetUniformTypeBaseAlignment(uniform.second);
            uint32_t stride = GetUniformTypeStride(uniform.second);

            size_t padding = (alignment - (offset % alignment)) % alignment;
            offset += padding;

            std::get<1>(variable) = offset * sizeof(uint32_t);

            offset += stride;
            uniforms[uniform.first] = variable;
        }
        storage_size = offset * sizeof(uint32_t);
        storage_dirty = true;
        if (storage_size > 0) {
            blast::GfxBufferDesc buffer_desc{};
            buffer_desc.size = storage_size;
            buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
            buffer_desc.res_usage = blast::RESOURCE_USAGE_UNIFORM_BUFFER;
            material_ub = std::shared_ptr<blast::GfxBuffer>(gEngine.GetRenderer()->GetDevice()->CreateBuffer(buffer_desc));
        } else {
            material_ub = nullptr;
        }

        // Texture
        textures = material->textures;
        TextureSlot texture_slot = 0;
        for (auto& texture : textures) {
            texture_slot_map[texture.first] = texture_slot;
            texture_group[texture_slot] = nullptr;
            texture_slot++;
        }

        // sampler
        samplers = material->samplers;
        SamplerSlot sampler_slot = 0;
        for (auto& sampler : samplers) {
            sampler_slot_map[sampler] = sampler_slot;
            blast::GfxSamplerDesc sampler_desc;
            sampler_group[sampler_slot] = sampler_desc;
            sampler_slot++;
        }
    }

    MaterialInstance::~MaterialInstance() {
        int m = 2;
    }

    blast::GfxBuffer* MaterialInstance::GetUniformBuffer() {
        if (!material_ub) {
            return nullptr;
        }

        return material_ub.get();
    }

    void MaterialInstance::SetBool(const std::string& name, const bool& value) {
        auto iter = uniforms.find(name);
        if (iter != uniforms.end()) {
            storage_dirty = true;
            memcpy(storage + std::get<1>(iter->second), &value, sizeof(bool));
        }
    }

    void MaterialInstance::SetFloat(const std::string& name, const float& value) {
        auto iter = uniforms.find(name);
        if (iter != uniforms.end()) {
            storage_dirty = true;
            memcpy(storage + std::get<1>(iter->second), &value, sizeof(float));
        }
    }

    void MaterialInstance::SetFloat2(const std::string& name, const glm::vec2& value) {
        auto iter = uniforms.find(name);
        if (iter != uniforms.end()) {
            storage_dirty = true;
            memcpy(storage + std::get<1>(iter->second), &value, sizeof(glm::vec2));
        }
    }

    void MaterialInstance::SetFloat3(const std::string& name, const glm::vec3& value) {
        auto iter = uniforms.find(name);
        if (iter != uniforms.end()) {
            storage_dirty = true;
            memcpy(storage + std::get<1>(iter->second), &value, sizeof(glm::vec3));
        }
    }

    void MaterialInstance::SetFloat4(const std::string& name, const glm::vec4& value) {
        auto iter = uniforms.find(name);
        if (iter != uniforms.end()) {
            storage_dirty = true;
            uint32_t offset = std::get<1>(iter->second);
            memcpy(storage + offset, &value, sizeof(glm::vec4));
        }
    }

    void MaterialInstance::SetMat4(const std::string& name, const glm::mat4& value) {
        auto iter = uniforms.find(name);
        if (iter != uniforms.end()) {
            storage_dirty = true;
            memcpy(storage + std::get<1>(iter->second), &value, sizeof(glm::mat4));
        }
    }

    void MaterialInstance::SetTexture(const std::string& name, std::shared_ptr<blast::GfxTexture> texture) {
        auto iter = texture_slot_map.find(name);
        if (iter != texture_slot_map.end()) {
            texture_group[iter->second] = texture;
        }
    }

    void MaterialInstance::SetSampler(const std::string& name, const blast::GfxSamplerDesc& sampler_desc) {
        auto iter = sampler_slot_map.find(name);
        if (iter != sampler_slot_map.end()) {
            sampler_group[iter->second] = sampler_desc;
        }
    }

    void MaterialInstance::SetScissor(float x, float y, float w, float h) {
        scissor.x = x;
        scissor.y = y;
        scissor.z = w;
        scissor.w = h;
    }

}