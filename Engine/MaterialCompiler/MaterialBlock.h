#pragma once

#include <vector>
#include <unordered_map>

namespace gear {
    enum class UniformType {
        BOOL,
        BOOL2,
        BOOL3,
        BOOL4,
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        INT,
        INT2,
        INT3,
        INT4,
        UINT,
        UINT2,
        UINT3,
        UINT4,
        MAT3,
        MAT4
    };

    class UniformBlockLayout {
    public:
        UniformBlockLayout() = default;
        void begin();
        void end();
        void add(const std::string& name, uint32_t size);
        uint16_t getUniformOffset(const std::string& name, uint32_t index);
    private:
        struct Entry {
            std::string name;
            uint32_t size;
            UniformType type;
        };

        struct UniformInfo {
            uint16_t offset;
            uint8_t stride;
            uint32_t size;
            UniformType type;
        };

        std::string mName;
        std::vector<Entry> mEntries;
        std::unordered_map<std::string, UniformInfo> mInfos;
    };

    class UniformBlock {
    public:
        UniformBlock(uint32_t size);

    private:
        uint8_t mStorage[96];
        uint32_t mSize = 0;
    };

    enum class SamplerType {
        SAMPLER_2D,
        SAMPLER_2D_ARRAY,
        SAMPLER_CUBEMAP,
        SAMPLER_3D,
    };

    class SamplerBlockLayout {
    public:
        SamplerBlockLayout() = default;
        void begin();
        void end();
        void add(const std::string& name, SamplerType type);
    private:
        struct Entry {
            std::string name;
            SamplerType type;
        };
        struct SamplerInfo {
            uint16_t offset;
            SamplerType type;
        };
        std::string mName;
        std::vector<Entry> mEntries;
        std::unordered_map<std::string, SamplerInfo> mInfos;
    };

    class SamplerBlock {
    public:

    };

    // 获取公共布局接口

}