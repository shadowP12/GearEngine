#ifndef RHI_RENDER_STATE_H
#define RHI_RENDER_STATE_H
#include "RHIDefine.h"
#include <string>
 enum VertexSemantic
{
    SEMANTIC_UNDEFINED = 0,
    SEMANTIC_POSITION,
    SEMANTIC_NORMAL,
    SEMANTIC_COLOR,
    SEMANTIC_TANGENT,
    SEMANTIC_BITANGENT,
    SEMANTIC_JOINTS,
    SEMANTIC_WEIGHTS,
    SEMANTIC_TEXCOORD0,
    SEMANTIC_TEXCOORD1,
    SEMANTIC_TEXCOORD2,
    SEMANTIC_TEXCOORD3,
    SEMANTIC_TEXCOORD4,
    SEMANTIC_TEXCOORD5,
    SEMANTIC_TEXCOORD6,
    SEMANTIC_TEXCOORD7,
    SEMANTIC_TEXCOORD8,
    SEMANTIC_TEXCOORD9,
};

enum VertexAttribRate
{
    VERTEX_ATTRIB_RATE_VERTEX = 0,
    VERTEX_ATTRIB_RATE_INSTANCE = 1,
    VERTEX_ATTRIB_RATE_COUNT,
};

struct VertexAttrib
{
    VertexSemantic semantic;
    std::string name;
    VkFormat format;
    uint32_t binding;
    uint32_t location;
    uint32_t offset;
    VertexAttribRate  rate;
};

struct VertexLayout
{
    uint32_t attribCount;
    VertexAttrib attribs[8];
};

enum BlendStateTargets
{
    BLEND_STATE_TARGET_0 = 0x1,
    BLEND_STATE_TARGET_1 = 0x2,
    BLEND_STATE_TARGET_2 = 0x4,
    BLEND_STATE_TARGET_3 = 0x8,
    BLEND_STATE_TARGET_4 = 0x10,
    BLEND_STATE_TARGET_5 = 0x20,
    BLEND_STATE_TARGET_6 = 0x40,
    BLEND_STATE_TARGET_7 = 0x80,
    BLEND_STATE_TARGET_ALL = 0xFF,
};
MAKE_ENUM_FLAG(uint32_t, BlendStateTargets)

struct BlendState
{
    VkBlendFactor srcFactors[4] = { VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE };
    VkBlendFactor dstFactors[4] = { VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO };
    VkBlendFactor srcAlphaFactors[4] = { VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE };
    VkBlendFactor dstAlphaFactors[4] = { VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO };
    VkBlendOp blendModes[4] = { VK_BLEND_OP_ADD, VK_BLEND_OP_ADD, VK_BLEND_OP_ADD, VK_BLEND_OP_ADD };
    VkBlendOp blendAlphaModes[4] = { VK_BLEND_OP_ADD, VK_BLEND_OP_ADD, VK_BLEND_OP_ADD, VK_BLEND_OP_ADD };;
    BlendStateTargets renderTargetMask = BLEND_STATE_TARGET_ALL;
};

struct DepthStencilState
{
    // todo Stencil
    bool depthTest = true;
    bool depthWrite = true;
    VkCompareOp depthFunc = VK_COMPARE_OP_LESS;
};

struct RasterizerState
{
    bool multiSample = false;
    bool scissor = true;
    bool depthClampEnable = false;
    VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
};

#endif