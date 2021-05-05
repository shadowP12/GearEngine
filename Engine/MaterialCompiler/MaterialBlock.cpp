#include "MaterialBlock.h"

namespace gear {
    static uint8_t getUniformTypeBaseAlignment(UniformType type) {
        switch (type) {
            case UniformType::BOOL:
            case UniformType::FLOAT:
            case UniformType::INT:
            case UniformType::UINT:
                return 1;
            case UniformType::BOOL2:
            case UniformType::FLOAT2:
            case UniformType::INT2:
            case UniformType::UINT2:
                return 2;
            case UniformType::BOOL3:
            case UniformType::BOOL4:
            case UniformType::FLOAT3:
            case UniformType::FLOAT4:
            case UniformType::INT3:
            case UniformType::INT4:
            case UniformType::UINT3:
            case UniformType::UINT4:
            case UniformType::MAT3:
            case UniformType::MAT4:
                return 4;
        }
    }

    static uint8_t getUniformTypeStride(UniformType type) {
        switch (type) {
            case UniformType::BOOL:
            case UniformType::INT:
            case UniformType::UINT:
            case UniformType::FLOAT:
                return 1;
            case UniformType::BOOL2:
            case UniformType::INT2:
            case UniformType::UINT2:
            case UniformType::FLOAT2:
                return 2;
            case UniformType::BOOL3:
            case UniformType::INT3:
            case UniformType::UINT3:
            case UniformType::FLOAT3:
                return 3;
            case UniformType::BOOL4:
            case UniformType::INT4:
            case UniformType::UINT4:
            case UniformType::FLOAT4:
                return 4;
            case UniformType::MAT3:
                return 12;
            case UniformType::MAT4:
                return 16;
        }
    }

    void UniformBlockLayout::begin() {
        mEntries.clear();
        mInfos.clear();
    }

    void UniformBlockLayout::add(const std::string& name, uint32_t size) {
        Entry entry = {};
        entry.name = name;
        entry.size = size;
        mEntries.push_back(entry);
    }

    void  UniformBlockLayout::end() {
        uint16_t offset = 0;
        for (int i = 0; i < mEntries.size(); ++i) {
            UniformInfo info = {};
            size_t alignment = getUniformTypeBaseAlignment(mEntries[i].type);
            uint8_t stride = getUniformTypeStride(mEntries[i].type);
            if (mEntries[i].size > 1) {
                alignment = (alignment + 3) & ~3;
                stride = (stride + uint8_t(3)) & ~uint8_t(3);
            }

            size_t padding = (alignment - (offset % alignment)) % alignment;
            offset += padding;

            info.offset = offset;
            info.stride = stride;
            info.size = mEntries[i].size;
            info.type = mEntries[i].type;

            offset += stride * mEntries[i].size;
            mInfos[mEntries[i].name] = info;
        }
    }

    uint16_t UniformBlockLayout::getUniformOffset(const std::string& name, uint32_t index) {
        auto const& pos = mInfos.find(name);
        if (pos == mInfos.end()) {
            return -1;
        }
        return (mInfos[name].offset + mInfos[name].stride * index) * sizeof(uint32_t);
    }

    void SamplerBlockLayout::begin() {
        mEntries.clear();
        mInfos.clear();
    }

    void SamplerBlockLayout::add(const std::string& name, SamplerType type) {
        Entry entry = {};
        entry.name = name;
        entry.type = type;
        mEntries.push_back(entry);
    }

    void SamplerBlockLayout::end() {
        for (int i = 0; i < mEntries.size(); ++i) {
            SamplerInfo info = {};
            info.offset = i;
            info.type = mEntries[i].type;

            mInfos[mEntries[i].name] = info;
        }
    }

}