#pragma once
#include "Core/GearDefine.h"
#include "Math/Math.h"
#include "Resource/Resource.h"
#include <Blast/Gfx/GfxSampler.h>
#include <Blast/Gfx/GfxPipeline.h>
#include <Blast/Utility/ShaderCompiler.h>
#include <string>
#include <unordered_map>

namespace Blast {
    class GfxShader;
}

namespace gear {
    class Texture;

    // 着色模型
    enum class Shading {
        UNLIT,
        LIT,
    };

    // 材质应用的范围
    enum class MaterialDomain {
        SURFACE = 0,
        POST_PROCESS = 1,
    };

    // 混合模式
    enum class BlendingMode {
        BLENDING_MODE_OPAQUE,
        BLENDING_MODE_TRANSPARENT,
        BLENDING_MODE_MASKED,
    };

    // 材质属性
    static constexpr size_t MATERIAL_PROPERTIES_COUNT = 1;
    enum class MaterialProperty {
        BASE_COLOR,
    };

    // 材质的变体参数
    static constexpr size_t MATERIAL_VARIANT_COUNT = 2;
    struct MaterialVariant {
    public:
        MaterialVariant(uint8_t key) : key(key) { }
        static constexpr uint8_t SKINNING_OR_MORPHING   = 0x01; // 使用GPU蒙皮
        static constexpr uint8_t VERTEX_MASK = SKINNING_OR_MORPHING;
        static constexpr uint8_t FRAGMENT_MASK = 0;

        inline bool hasSkinningOrMorphing() const noexcept { return key & SKINNING_OR_MORPHING; }

        inline void setSkinning(bool v) noexcept { set(v, SKINNING_OR_MORPHING); }

        static constexpr uint8_t filterVariantVertex(uint8_t variantKey) noexcept {
            // 过滤掉不需要的顶点变体
            return variantKey & VERTEX_MASK;
        }

        static constexpr uint8_t filterVariantFragment(uint8_t variantKey) noexcept {
            // 过滤掉不需要的片段变体
            return variantKey & FRAGMENT_MASK;
        }
    private:
        void set(bool v, uint8_t mask) noexcept {
            key = (key & ~mask) | (v ? mask : uint8_t(0));
        }

        uint8_t key = 0;
    };

    class MaterialInstance;

    // TODO: Material ID
    class Material : public Resource {
    public:
        class Builder {
        public:
            Builder() = default;

            ~Builder() = default;

            void shading(Shading);

            void blendingMode(BlendingMode);

            void depthWrite(bool);

            Material* build();

        private:
            friend class Material;
            Shading mShading = Shading::UNLIT;
            BlendingMode mBlendingMode = BlendingMode::BLENDING_MODE_OPAQUE;
            bool mDepthWrite = false;
            std::unordered_map<uint8_t, Blast::GfxShader*> mVertShaderCache;
            std::unordered_map<uint8_t, Blast::GfxShader*> mFragShaderCache;
        };

        ~Material();

        Blast::GfxShader* getVertShader(uint8_t variant);

        Blast::GfxShader* getFragShader(uint8_t variant);

        MaterialInstance* createInstance();

        Blast::GfxShaderVariable getVariable(const std::string& name);

    private:
        Material(Builder*);

    protected:
        friend class Renderer;
        friend class MaterialCompiler;
        friend class MaterialInstance;
        Shading mShading;
        BlendingMode mBlendingMode;
        bool mDepthWrite = false;
        Blast::GfxBlendState mBlendState;
        Blast::GfxDepthState mDepthState;
        Blast::GfxRasterizerState mRasterizerState;
        std::vector<Blast::GfxShaderResource> mResources;
        std::vector<Blast::GfxShaderVariable> mVariables;
        std::unordered_map<uint8_t, Blast::GfxShader*> mVertShaderCache;
        std::unordered_map<uint8_t, Blast::GfxShader*> mFragShaderCache;
    };

    struct SamplerInfo {
        Texture* texture;
        Blast::GfxSamplerDesc params;
    };

    class UniformBuffer;

    // TODO: MaterialInsrance ID
    class MaterialInstance {
    public:
        ~MaterialInstance();

        Material* getMaterial() { return mMaterial; }

        void setParameter(const char* name, void* data, uint32_t offset, uint32_t size);

        void setParameter(const char* name, Texture* texture, Blast::GfxSamplerDesc params);
    private:
        MaterialInstance(Material* material);
    private:
        friend class Renderer;
        friend class Material;
        Material* mMaterial = nullptr;
        uint8_t mStorage[128];
        uint32_t mUniformBufferSize;
        bool mBufferDirty = false;
        UniformBuffer* mUniformBuffer;
        std::unordered_map<uint32_t, SamplerInfo> mSamplerGroup;
    };
}