#include "Resource/Material.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
#include "GearEngine.h"

#include <Blast/Gfx/GfxDefine.h>
#include <Blast/Gfx/GfxDevice.h>

namespace gear {
    uint32_t Material::global_material_id = 0;

    // 过滤掉不需要的顶点变体
    constexpr MaterialVariant::Key MaterialVariant::FilterVariantVertex(Key variant) noexcept {
        return variant & VERTEX_MASK;
    }

    // 过滤掉不需要的片段变体
    constexpr MaterialVariant::Key MaterialVariant::FilterVariantFragment(Key variant) noexcept {
        return variant & FRAGMENT_MASK;
    }

    // 通过着色模型过滤掉不需要的变体
    constexpr MaterialVariant::Key MaterialVariant::FilterVariantShadingMode(Key variant, ShadingModel shading_model) noexcept {
        if (shading_model == SHADING_MODEL_UNLIT) {
            Set(variant, false, DYNAMIC_LIGHTING);
            Set(variant, false, DIRECTIONAL_LIGHTING);
        }
        return variant;
    }

    // 通过顶点布局模型过滤掉不需要的片段变体
    constexpr MaterialVariant::Key MaterialVariant::FilterVariantVertexLayout(Key variant, VertexLayoutType vertex_layout_type) noexcept {
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

    bool Material::ShaderEq::operator()(const ShaderKey& key1, const ShaderKey& key2) const {
        if (key1.variant != key2.variant) return false;
        if (key1.vertex_layout_type != key2.vertex_layout_type) return false;
        return true;
    }

    void Material::Builder::SetShadingModel(ShadingModel shading_model) {
        this->shading_model = shading_model;
    }

    void Material::Builder::SetBlendState(BlendStateType blend_state) {
        this->blend_state = blend_state;
    }

    void Material::Builder::AddVertShader(MaterialVariant::Key key, VertexLayoutType vertex_layout_type, blast::GfxShader* shader) {
        ShaderKey shader_key;
        shader_key.variant = key;
        shader_key.vertex_layout_type = vertex_layout_type;
        vert_shader_cache[shader_key] = shader;
    }

    void Material::Builder::AddFragShader(MaterialVariant::Key key, VertexLayoutType vertex_layout_type, blast::GfxShader* shader) {
        ShaderKey shader_key;
        shader_key.variant = key;
        shader_key.vertex_layout_type = vertex_layout_type;
        frag_shader_cache[shader_key] = shader;
    }

    void Material::Builder::AddUniform(const std::string& name, const blast::UniformType& type) {
        uniforms[name] = type;
    }

    void Material::Builder::AddTexture(const std::string& name, const blast::TextureDimension& dim) {
        textures[name] = dim;
    }

    void Material::Builder::AddSampler(const std::string& name) {
        samplers.push_back(name);
    }

    Material * Material::Builder::Build() {
        return new Material(this);
    }

    Material::Material(Builder* builder) {
        shading_model = builder->shading_model;
        blend_state = builder->blend_state;
        uniforms = builder->uniforms;
        textures = builder->textures;
        samplers = builder->samplers;
        vert_shader_cache = builder->vert_shader_cache;
        frag_shader_cache = builder->frag_shader_cache;
        material_id = global_material_id;
        global_material_id++;
    }

    Material::~Material() {
        blast::GfxDevice* device = gEngine.GetDevice();
        for (auto& vs : vert_shader_cache) {
            device->DestroyShader(vs.second);
        }

        for (auto& fs : frag_shader_cache) {
            device->DestroyShader(fs.second);
        }
    }

    blast::GfxShader* Material::GetVertShader(MaterialVariant::Key variant, VertexLayoutType vertex_layout_type) {
        MaterialVariant::Key key = MaterialVariant::FilterVariantVertex(variant);
        key = MaterialVariant::FilterVariantShadingMode(key, shading_model);
        key = MaterialVariant::FilterVariantVertexLayout(key, vertex_layout_type);

        ShaderKey shader_key;
        shader_key.variant = key;
        shader_key.vertex_layout_type = vertex_layout_type;

        return vert_shader_cache[shader_key];
    }

    blast::GfxShader* Material::GetFragShader(MaterialVariant::Key variant, VertexLayoutType vertex_layout_type) {
        uint8_t key = MaterialVariant::FilterVariantFragment(variant);
        key = MaterialVariant::FilterVariantShadingMode(key, shading_model);
        key = MaterialVariant::FilterVariantVertexLayout(key, vertex_layout_type);

        ShaderKey shader_key;
        shader_key.variant = key;
        shader_key.vertex_layout_type = vertex_layout_type;

        return frag_shader_cache[shader_key];
    }

    MaterialInstance* Material::CreateInstance() {
        MaterialInstance* mi = new MaterialInstance(this);
        mi->material_instance_id = current_material_instance_id;
        current_material_instance_id++;
        return mi;
    }

    MaterialInstance::MaterialInstance(Material* material) {
        this->material = material;

        // uniform
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
            material_ub = new UniformBuffer(storage_size);
        } else {
            material_ub = nullptr;
        }

        // texture
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
        SAFE_DELETE(material_ub);
    }

    UniformBuffer* MaterialInstance::GetUniformBuffer() {
        if (!material_ub) {
            return nullptr;
        }

        return material_ub;
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

    void MaterialInstance::SetTexture(const std::string& name, Texture* texture) {
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