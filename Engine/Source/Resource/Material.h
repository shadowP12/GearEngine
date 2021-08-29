#pragma once
#include "Core/GearDefine.h"
#include "Math/Math.h"
#include "Renderer/RenderData.h"
#include <Blast/Gfx/GfxSampler.h>
#include <string>
#include <set>
#include <unordered_map>
#include <tuple>

namespace blast {
    class GfxShader;
}

namespace gear {
    class Texture;

    // 材质属性
    static constexpr size_t MATERIAL_PROPERTIES_COUNT = 1;
    enum class MaterialProperty {
        BASE_COLOR,
    };

    // 材质的变体参数
    static constexpr uint32_t MATERIAL_VARIANT_COUNT = 32;
    struct MaterialVariant {
    public:
        typedef uint32_t Key;
        MaterialVariant(Key key) : key(key) { }
        static constexpr Key DIRECTIONAL_LIGHTING   = 0x01; // 方向光
        static constexpr Key DYNAMIC_LIGHTING       = 0x02; // 动态光
        static constexpr Key SHADOW_RECEIVER        = 0x04; // 接收阴影
        static constexpr Key SKINNING_OR_MORPHING   = 0x08; // GPU蒙皮
        static constexpr Key DEPTH                  = 0x10; // 深度

        static constexpr Key VERTEX_MASK =  DIRECTIONAL_LIGHTING |
                                            DYNAMIC_LIGHTING |
                                            SHADOW_RECEIVER |
                                            SKINNING_OR_MORPHING |
                                            DEPTH;

        static constexpr Key FRAGMENT_MASK =    DIRECTIONAL_LIGHTING |
                                                DYNAMIC_LIGHTING |
                                                SHADOW_RECEIVER |
                                                DEPTH;

        static constexpr Key DEPTH_MASK =   DIRECTIONAL_LIGHTING |
                                            DYNAMIC_LIGHTING |
                                            SHADOW_RECEIVER |
                                            DEPTH;

        inline bool HasSkinningOrMorphing() const noexcept { return key & SKINNING_OR_MORPHING; }
        inline bool HasDirectionalLighting() const noexcept { return key & DIRECTIONAL_LIGHTING; }
        inline bool HasDynamicLighting() const noexcept { return key & DYNAMIC_LIGHTING; }
        inline bool HasShadowReceiver() const noexcept { return key & SHADOW_RECEIVER; }

        inline void SetSkinning(bool v) noexcept { Set(v, SKINNING_OR_MORPHING); }
        inline void SetDirectionalLighting(bool v) noexcept { Set(v, DIRECTIONAL_LIGHTING); }
        inline void SetDynamicLighting(bool v) noexcept { Set(v, DYNAMIC_LIGHTING); }
        inline void SetShadowReceiver(bool v) noexcept { Set(v, SHADOW_RECEIVER); }

        // 过滤掉不需要的顶点变体
        static constexpr Key FilterVariantVertex(Key variant) noexcept {
            return variant & VERTEX_MASK;
        }

        // 过滤掉不需要的片段变体
        static constexpr Key FilterVariantFragment(Key variant) noexcept {
            return variant & FRAGMENT_MASK;
        }

    private:
        void Set(bool v, Key mask) noexcept {
            key = (key & ~mask) | (v ? mask : Key(0));
        }

        Key key = 0;
    };

    class MaterialInstance;

    class Material {
    public:
        class Builder {
        public:
            Builder() = default;

            ~Builder() = default;

            void SetShadingModel(ShadingModel shading_model);

            void SetBlendingMode(BlendingMode blending_model);

            void AddVertShader(MaterialVariant::Key, blast::GfxShader*);

            void AddFragShader(MaterialVariant::Key, blast::GfxShader*);

            void AddUniform(const std::string& name, const blast::UniformType& type);

            void AddSampler(const std::string& name, const blast::TextureDimension& dim);

            Material* Build();

        private:
            friend class Material;
            friend class MaterialCompiler;
            RenderState _render_state;
            std::unordered_map<std::string, blast::UniformType> _uniforms;
            std::unordered_map<std::string, blast::TextureDimension> _samplers;
            std::unordered_map<MaterialVariant::Key, blast::GfxShader*> _vert_shader_cache;
            std::unordered_map<MaterialVariant::Key, blast::GfxShader*> _frag_shader_cache;
        };

        ~Material();

        blast::GfxShader* GetVertShader(MaterialVariant::Key variant);

        blast::GfxShader* GetFragShader(MaterialVariant::Key variant);

        RenderState GetRenderState() { return _render_state; }

        MaterialInstance* CreateInstance();

    private:
        Material(Builder*);

    protected:
        friend class Renderer;
        friend class MaterialCompiler;
        friend class MaterialInstance;
        RenderState _render_state;
        std::unordered_map<std::string, blast::UniformType> _uniforms;
        std::unordered_map<std::string, blast::TextureDimension> _samplers;
        std::unordered_map<MaterialVariant::Key, blast::GfxShader*> _vert_shader_cache;
        std::unordered_map<MaterialVariant::Key, blast::GfxShader*> _frag_shader_cache;
    };

    class UniformBuffer;
    class MaterialInstance {
    public:
        typedef uint32_t TextureSlot;

        ~MaterialInstance();

        Material* GetMaterial() { return _material; }

        UniformBuffer* GetUniformBuffer();

        const std::unordered_map<TextureSlot, std::pair<Texture*, blast::GfxSamplerDesc>>& GetGfxSamplerGroup() { return _sampler_group; }

        void SetBool(const std::string& name, const bool& value);

        void SetFloat(const std::string& name, const float& value);

        void SetFloat2(const std::string& name, const glm::vec2& value);

        void SetFloat3(const std::string& name, const glm::vec3& value);

        void SetFloat4(const std::string& name, const glm::vec4& value);

        void SetMat4(const std::string& name, const glm::mat4& value);

        void SetTexture(const std::string& name, Texture* texture, const blast::GfxSamplerDesc& sampler_desc);

        void SetScissor(float x, float y, float w, float h);

    private:
        MaterialInstance(Material* material);

    private:
        friend class Renderer;
        friend class Material;
        Material* _material = nullptr;

        // uniform
        uint8_t _storage[128];
        uint32_t _storage_size;
        bool _storage_dirty;
        UniformBuffer* _material_ub;
        std::unordered_map<std::string, std::tuple<blast::UniformType, uint32_t>> _uniforms;

        // texture
        std::unordered_map<std::string, blast::TextureDimension> _samplers;
        std::unordered_map<std::string, TextureSlot> _slot_map;
        std::unordered_map<TextureSlot, std::pair<Texture*, blast::GfxSamplerDesc>> _sampler_group;
    };
}