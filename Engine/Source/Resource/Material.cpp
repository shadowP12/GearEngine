#include "Resource/Material.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
namespace gear {
    void Material::Builder::shading(Shading shading) {
        mShading = shading;
    }

    void Material::Builder::blendingMode(BlendingMode blendingMode) {
        mBlendingMode = blendingMode;
    }

    void Material::Builder::depthWrite(bool enable) {
        mDepthWrite = enable;
    }

    Material * Material::Builder::build() {
        return new Material(this);
    }

    Material::Material(Builder* builder) {
        mShading = builder->mShading;
        mBlendingMode = builder->mBlendingMode;
        mDepthWrite = builder->mDepthWrite;

        if (mBlendingMode == BlendingMode::BLENDING_MODE_OPAQUE) {
            mBlendState.srcFactors[0] = Blast::BLEND_ONE;
            mBlendState.dstFactors[0] = Blast::BLEND_ZERO;
            mBlendState.srcAlphaFactors[0] = Blast::BLEND_ONE;
            mBlendState.dstAlphaFactors[0] = Blast::BLEND_ZERO;
            mBlendState.masks[0] = 0xf;
        } else if (mBlendingMode == BlendingMode::BLENDING_MODE_TRANSPARENT) {
            // 预乘使用的混合方程
            mBlendState.srcFactors[0] = Blast::BLEND_ONE;
            mBlendState.dstFactors[0] = Blast::BLEND_ONE_MINUS_SRC_ALPHA;
            mBlendState.srcAlphaFactors[0] = Blast::BLEND_ONE;
            mBlendState.dstAlphaFactors[0] = Blast::BLEND_ONE_MINUS_SRC_ALPHA;
            mBlendState.masks[0] = 0xf;
        } else if (mBlendingMode == BlendingMode::BLENDING_MODE_MASKED) {
            mBlendState.srcFactors[0] = Blast::BLEND_ONE;
            mBlendState.dstFactors[0] = Blast::BLEND_ZERO;
            mBlendState.srcAlphaFactors[0] = Blast::BLEND_ZERO;
            mBlendState.dstAlphaFactors[0] = Blast::BLEND_ONE;
            mBlendState.masks[0] = 0xf;
        }

        if (mBlendingMode == BlendingMode::BLENDING_MODE_TRANSPARENT) {
            // 半透材质不应该写入深度缓存
            mDepthWrite = false;
        }
        mDepthState.depthTest = true;
        mDepthState.depthWrite = mDepthWrite;
    }

    Material::~Material() {
        for (auto& vs : mVertShaderCache) {
            SAFE_DELETE(vs.second);
        }

        for (auto& fs : mFragShaderCache) {
            SAFE_DELETE(fs.second);
        }
    }

    Blast::GfxShader* Material::getVertShader(uint8_t variant) {
        uint8_t key = MaterialVariant::filterVariantVertex(variant);
        return mVertShaderCache[key];
    }

    Blast::GfxShader* Material::getFragShader(uint8_t variant) {
        uint8_t key = MaterialVariant::filterVariantFragment(variant);
        return mFragShaderCache[key];
    }

    Blast::GfxShaderVariable Material::getVariable(const std::string& name) {
        for (int i = 0; i < mVariables.size(); ++i) {
            if (mVariables[i].name == name) {
                return mVariables[i];
            }
        }
        return Blast::GfxShaderVariable();
    }

    MaterialInstance* Material::createInstance() {
        return new MaterialInstance(this);
    }

    MaterialInstance::MaterialInstance(Material* material) {
        mMaterial = material;
        mUniformBufferSize = 0;
        for (int i = 0; i < mMaterial->mVariables.size(); i++) {
            mUniformBufferSize += mMaterial->mVariables[i].size;
        }
        if (mUniformBufferSize > 0) {
            mUniformBuffer = new UniformBuffer(mUniformBufferSize);
        } else {
            mUniformBuffer = nullptr;
        }
    }

    MaterialInstance::~MaterialInstance() {
        SAFE_DELETE(mUniformBuffer);
    }

    void MaterialInstance::setParameter(const char* name, void* data, uint32_t offset, uint32_t size) {
        for (int i = 0; i < mMaterial->mVariables.size(); i++) {
            if (mMaterial->mVariables[i].name == name) {
                mBufferDirty = true;
                memcpy(mStorage + mMaterial->mVariables[i].offset + offset, data, size);
                break;
            }
        }
        // TODO: uniform buffer更新可以合批处理
        mUniformBuffer->update(mStorage, 0, mUniformBufferSize);
    }

    void MaterialInstance::setParameter(const char* name, Texture* texture, Blast::GfxSamplerDesc params) {
        for (int i = 0; i < mMaterial->mResources.size(); ++i) {
            if (mMaterial->mResources[i].name == name) {
                SamplerInfo info;
                info.texture = texture;
                info.params = params;
                mSamplerGroup[mMaterial->mResources[i].reg] = info;
                break;
            }
        }
    }

}