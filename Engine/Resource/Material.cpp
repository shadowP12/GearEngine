#include "Resource/Material.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
namespace gear {
    Material::Material() {
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

    MaterialInstance* Material::createInstance() {
        return new MaterialInstance(this);
    }

    MaterialInstance::MaterialInstance(Material* material) {
        mMaterial = material;
        mUniformBufferSize = 0;
        for (int i = 0; i < mMaterial->mVariables.size(); i++) {
            mUniformBufferSize += mMaterial->mVariables[i].size;
        }
        mUniformBuffer = new UniformBuffer(mUniformBufferSize);
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
    }

    void MaterialInstance::setParameter(const char* name, Texture* texture, Blast::GfxSamplerDesc params) {
        for (int i = 0; i < mMaterial->mResources.size(); ++i) {
            if (mMaterial->mResources[i].name == name) {
                SamplerInfo info;
                info.texture = texture;
                info.params = params;
                mSamplerGroup[mMaterial->mResources[i].set] = info;
                break;
            }
        }
    }

}