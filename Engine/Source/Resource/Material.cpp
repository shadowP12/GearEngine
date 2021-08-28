#include "Resource/Material.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
namespace gear {
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


    void Material::Builder::SetShadingModel(ShadingModel shading_model) {
        _render_state.shading_model = shading_model;
    }

    void Material::Builder::SetBlendingMode(BlendingMode blending_model) {
        _render_state.blending_mode = blending_model;
    }

    void Material::Builder::AddVertShader(MaterialVariant::Key key, blast::GfxShader* shader) {
        _vert_shader_cache[key] = shader;
    }

    void Material::Builder::AddFragShader(MaterialVariant::Key key, blast::GfxShader* shader) {
        _frag_shader_cache[key] = shader;
    }

    void Material::Builder::AddSampler(const std::string& name, const blast::TextureDimension& dim) {
        _samplers[name] = dim;
    }

    void Material::Builder::AddUniform(const std::string& name, const blast::UniformType& type) {
        _uniforms[name] = type;
    }

    Material * Material::Builder::Build() {
        return new Material(this);
    }

    Material::Material(Builder* builder) {
        _render_state = builder->_render_state;
        _samplers = builder->_samplers;
        _uniforms = builder->_uniforms;
        _vert_shader_cache = builder->_vert_shader_cache;
        _frag_shader_cache = builder->_frag_shader_cache;
    }

    Material::~Material() {
        Renderer* renderer = gEngine.GetRenderer();
        for (auto& vs : _vert_shader_cache) {
            renderer->Destroy(vs.second);
        }

        for (auto& fs : _frag_shader_cache) {
            renderer->Destroy(fs.second);
        }
    }

    blast::GfxShader* Material::GetVertShader(MaterialVariant::Key variant) {
        uint8_t key = MaterialVariant::FilterVariantVertex(variant);
        return _vert_shader_cache[key];
    }

    blast::GfxShader* Material::GetFragShader(MaterialVariant::Key variant) {
        uint8_t key = MaterialVariant::FilterVariantFragment(variant);
        return _frag_shader_cache[key];
    }

    MaterialInstance* Material::CreateInstance() {
        return new MaterialInstance(this);
    }

    MaterialInstance::MaterialInstance(Material* material) {
        _material = material;

        // uniform
        uint32_t offset = 0;
        for (auto& uniform : material->_uniforms) {
            std::tuple<blast::UniformType, uint32_t> variable = {};
            std::get<0>(variable) = uniform.second;

            uint32_t alignment = GetUniformTypeBaseAlignment(uniform.second);
            uint32_t stride = GetUniformTypeStride(uniform.second);

            size_t padding = (alignment - (offset % alignment)) % alignment;
            offset += padding;

            std::get<1>(variable) = offset * sizeof(uint32_t);

            offset += stride;
            _uniforms[uniform.first] = variable;
        }
        _storage_size = offset * sizeof(uint32_t);
        _storage_dirty = true;
        if (_storage_size > 0) {
            _material_ub = new UniformBuffer(_storage_size);
        } else {
            _material_ub = nullptr;
        }

        // texture
        _samplers = material->_samplers;
        TextureSlot slot = 0;
        for (auto& sampler : _samplers) {
            _slot_map[sampler.first] = slot;
            blast::GfxSamplerDesc sampler_desc;
            _sampler_group[slot] = std::pair<Texture*, blast::GfxSamplerDesc>(nullptr, sampler_desc);
            slot++;
        }
    }

    MaterialInstance::~MaterialInstance() {
        SAFE_DELETE(_material_ub);
    }

    UniformBuffer* MaterialInstance::GetUniformBuffer() {
        if (!_material_ub) {
            return nullptr;
        }

        if (_storage_dirty) {
            _storage_dirty = false;
            _material_ub->Update(_storage, 0, _storage_size);
        }
        return _material_ub;
    }

    void MaterialInstance::SetBool(const std::string& name, const bool& value) {
        auto iter = _uniforms.find(name);
        if (iter != _uniforms.end()) {
            _storage_dirty = true;
            memcpy(_storage + std::get<1>(iter->second), &value, sizeof(bool));
        }
    }

    void MaterialInstance::SetFloat(const std::string& name, const float& value) {
        auto iter = _uniforms.find(name);
        if (iter != _uniforms.end()) {
            _storage_dirty = true;
            memcpy(_storage + std::get<1>(iter->second), &value, sizeof(float));
        }
    }

    void MaterialInstance::SetFloat2(const std::string& name, const glm::vec2& value) {
        auto iter = _uniforms.find(name);
        if (iter != _uniforms.end()) {
            _storage_dirty = true;
            memcpy(_storage + std::get<1>(iter->second), &value, sizeof(glm::vec2));
        }
    }

    void MaterialInstance::SetFloat3(const std::string& name, const glm::vec3& value) {
        auto iter = _uniforms.find(name);
        if (iter != _uniforms.end()) {
            _storage_dirty = true;
            memcpy(_storage + std::get<1>(iter->second), &value, sizeof(glm::vec3));
        }
    }

    void MaterialInstance::SetFloat4(const std::string& name, const glm::vec4& value) {
        auto iter = _uniforms.find(name);
        if (iter != _uniforms.end()) {
            _storage_dirty = true;
            uint32_t offset = std::get<1>(iter->second);
            memcpy(_storage + offset, &value, sizeof(glm::vec4));
        }
    }

    void MaterialInstance::SetMat4(const std::string& name, const glm::mat4& value) {
        auto iter = _uniforms.find(name);
        if (iter != _uniforms.end()) {
            _storage_dirty = true;
            memcpy(_storage + std::get<1>(iter->second), &value, sizeof(glm::mat4));
        }
    }

    void MaterialInstance::SetTexture(const std::string& name, Texture* texture, const blast::GfxSamplerDesc& sampler_desc) {
        auto iter = _slot_map.find(name);
        if (iter != _slot_map.end()) {
            _sampler_group[iter->second] = std::pair<Texture*, blast::GfxSamplerDesc>(texture, sampler_desc);
        }
    }
}