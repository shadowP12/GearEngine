#pragma once
#include "Core/GearDefine.h"
#include "Math/Math.h"
#include "Renderer/RenderData.h"
#include "Utility/Hash.h"

#include <Blast/Gfx/GfxDefine.h>
#include <Blast/Utility/ShaderCompiler.h>

#include <string>
#include <set>
#include <unordered_map>
#include <tuple>
#include <thread>
#include <mutex>

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
    static constexpr uint16_t MATERIAL_VARIANT_COUNT = 64;
    struct MaterialVariant {
    public:
        typedef uint16_t Key;
        MaterialVariant(Key key) : key(key) { }
        static constexpr Key DIRECTIONAL_LIGHTING   = 0x01; // 方向光
        static constexpr Key DYNAMIC_LIGHTING       = 0x02; // 动态光
        static constexpr Key SHADOW_RECEIVER        = 0x04; // 接收阴影
        static constexpr Key SKINNING_OR_MORPHING   = 0x08; // GPU蒙皮
        static constexpr Key DEPTH                  = 0x10; // 深度
        static constexpr Key IBL                    = 0x20; // ibl

        static constexpr Key VERTEX_MASK =  DIRECTIONAL_LIGHTING |
                                            DYNAMIC_LIGHTING |
                                            SHADOW_RECEIVER |
                                            SKINNING_OR_MORPHING |
                                            DEPTH;

        static constexpr Key FRAGMENT_MASK =    DIRECTIONAL_LIGHTING |
                                                DYNAMIC_LIGHTING |
                                                SHADOW_RECEIVER |
                                                DEPTH |
                                                IBL;

        static constexpr Key DEPTH_MASK =   DIRECTIONAL_LIGHTING |
                                            DYNAMIC_LIGHTING |
                                            SHADOW_RECEIVER |
                                            DEPTH;

        inline bool HasSkinningOrMorphing() const noexcept { return key & SKINNING_OR_MORPHING; }
        inline bool HasDirectionalLighting() const noexcept { return key & DIRECTIONAL_LIGHTING; }
        inline bool HasDynamicLighting() const noexcept { return key & DYNAMIC_LIGHTING; }
        inline bool HasShadowReceiver() const noexcept { return key & SHADOW_RECEIVER; }
        inline bool HasDepth() noexcept { return key & DEPTH; }
        inline bool HasIBL() noexcept { return key & IBL; }

        inline void SetSkinning(bool v) noexcept { Set(v, SKINNING_OR_MORPHING); }
        inline void SetDirectionalLighting(bool v) noexcept { Set(v, DIRECTIONAL_LIGHTING); }
        inline void SetDynamicLighting(bool v) noexcept { Set(v, DYNAMIC_LIGHTING); }
        inline void SetShadowReceiver(bool v) noexcept { Set(v, SHADOW_RECEIVER); }
        inline void SetIBL(bool v) noexcept { Set(v, IBL); }

        inline bool IsValidDepthVariant() noexcept {
            // 深度变体只有两种情况
            return key == DEPTH || key == (DEPTH | SKINNING_OR_MORPHING);
        }

        // 过滤掉不需要的顶点变体
        static Key FilterVariantVertex(Key variant);
        // 过滤掉不需要的片段变体
        static Key FilterVariantFragment(Key variant);

        // 通过着色模型过滤掉不需要的变体
        static Key FilterVariantShadingMode(Key variant, ShadingModel shading_model);

        // 通过顶点布局模型过滤掉不需要的片段变体
        static Key FilterVariantVertexLayout(Key variant, VertexLayoutType vertex_layout_type);

    private:
        void Set(bool v, Key mask) noexcept {
            key = (key & ~mask) | (v ? mask : Key(0));
        }

        static void Set(Key variant, bool v, Key mask) noexcept {
            variant = (variant & ~mask) | (v ? mask : Key(0));
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

            void SetBlendState(BlendStateType blend_state);

            void AddVertShader(MaterialVariant::Key, VertexLayoutType, blast::GfxShader*);

            void AddFragShader(MaterialVariant::Key, VertexLayoutType, blast::GfxShader*);

            void AddVertShaderCode(MaterialVariant::Key, VertexLayoutType, const std::string& code);

            void AddFragShaderCode(MaterialVariant::Key, VertexLayoutType, const std::string& code);

            void AddUniform(const std::string& name, const blast::UniformType& type);

            void AddTexture(const std::string& name, const blast::TextureDimension& dim);

            void AddSampler(const std::string& name);

            Material* Build();

        private:
            friend class Material;
            friend class MaterialCompiler;
            ShadingModel shading_model;
            BlendStateType blend_state;
            std::unordered_map<std::string, blast::UniformType> uniforms;
            std::unordered_map<std::string, blast::TextureDimension> textures;
            std::vector<std::string> samplers;
            std::unordered_map<std::size_t, blast::GfxShader*> vert_shader_cache;
            std::unordered_map<std::size_t, blast::GfxShader*> frag_shader_cache;
            std::unordered_map<std::size_t, std::string> vert_shader_code_cache;
            std::unordered_map<std::size_t, std::string> frag_shader_code_cache;
        };

        ~Material();

        uint32_t GetMaterialID() { return material_id; }

        blast::GfxShader* GetVertShader(MaterialVariant::Key variant, VertexLayoutType vertex_layout_type);

        blast::GfxShader* GetFragShader(MaterialVariant::Key variant, VertexLayoutType vertex_layout_type);

        ShadingModel GetShadingModel() { return shading_model; }

        BlendStateType GetBlendState() { return blend_state; }

        MaterialInstance* CreateInstance();

        std::unordered_map<std::string, blast::UniformType> GetUniformLayout() { return uniforms; }

        std::unordered_map<std::string, blast::TextureDimension> GetTextureLayout() { return textures; }

    private:
        Material(Builder*);

    protected:
        friend class MaterialCompiler;
        friend class MaterialInstance;
        std::unordered_map<std::size_t, blast::GfxShader*> vert_shader_cache;
        std::unordered_map<std::size_t, blast::GfxShader*> frag_shader_cache;
        std::unordered_map<std::size_t, std::string> vert_shader_code_cache;
        std::unordered_map<std::size_t, std::string> frag_shader_code_cache;
        std::unordered_map<std::size_t, bool> vert_shader_triggered_cache;
        std::unordered_map<std::size_t, bool> frag_shader_triggered_cache;
        std::mutex vs_cache_mutex;
        std::mutex fs_cache_mutex;
        static uint32_t global_material_id;
        uint32_t material_id = 0;
        uint32_t current_material_instance_id = 0;
        ShadingModel shading_model;
        BlendStateType blend_state;
        std::unordered_map<std::string, blast::UniformType> uniforms;
        std::unordered_map<std::string, blast::TextureDimension> textures;
        std::vector<std::string> samplers;
    };

    class UniformBuffer;
    class MaterialInstance {
    public:
        typedef uint32_t TextureSlot;
        typedef uint32_t SamplerSlot;

        MaterialInstance(Material* material);

        ~MaterialInstance();

        Material* GetMaterial() { return material; }

        uint32_t GetMaterialInstanceID() { return material_instance_id; }

        uint32_t GetStorageSize() { return storage_size; }

        uint8_t* GetStorage() { return storage; }

        bool IsStorageDirty() { return storage_dirty; }

        UniformBuffer* GetUniformBuffer();

        const std::unordered_map<std::string, std::tuple<blast::UniformType, uint32_t>>& GetUniformGroup() { return uniforms; }

        const std::unordered_map<TextureSlot, Texture*>& GetGfxTextureGroup() { return texture_group; }

        const std::unordered_map<SamplerSlot, blast::GfxSamplerDesc>& GetGfxSamplerGroup() { return sampler_group; }

        void SetBool(const std::string& name, const bool& value);

        void SetFloat(const std::string& name, const float& value);

        void SetFloat2(const std::string& name, const glm::vec2& value);

        void SetFloat3(const std::string& name, const glm::vec3& value);

        void SetFloat4(const std::string& name, const glm::vec4& value);

        void SetMat4(const std::string& name, const glm::mat4& value);

        void SetTexture(const std::string& name, Texture* texture);

        void SetSampler(const std::string& name, const blast::GfxSamplerDesc& sampler_desc);

        void SetScissor(float x, float y, float w, float h);

        glm::vec4 GetScissor() { return scissor; }

    private:
        friend Material;
        Material* material = nullptr;
        uint32_t material_instance_id = 0;

        // uniform
        uint8_t storage[128];
        uint32_t storage_size;
        bool storage_dirty;
        UniformBuffer* material_ub;
        std::unordered_map<std::string, std::tuple<blast::UniformType, uint32_t>> uniforms;

        // texture
        std::unordered_map<std::string, blast::TextureDimension> textures;
        std::unordered_map<std::string, TextureSlot> texture_slot_map;
        std::unordered_map<TextureSlot, Texture*> texture_group;

        // sampler
        std::vector<std::string> samplers;
        std::unordered_map<std::string, SamplerSlot> sampler_slot_map;
        std::unordered_map<SamplerSlot, blast::GfxSamplerDesc> sampler_group;

        glm::vec4 scissor = glm::vec4(0.0f, 0.0f, GEAR_INF, GEAR_INF);
    };
}