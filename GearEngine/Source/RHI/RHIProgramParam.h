#ifndef RHI_PROGRAM_PARAM
#define RHI_PROGRAM_PARAM

#include "RHIDefine.h"
#include <vector>
#include <map>
#include <string>

enum class RHIParamDataType
{
	FLOAT1 = 1,
	FLOAT2 = 2,
	FLOAT3 = 3,
	FLOAT4 = 4,
	MATRIX_2X2 = 11,
	MATRIX_2X3 = 12,
	MATRIX_2X4 = 13,
	MATRIX_3X2 = 14,
	MATRIX_3X3 = 15,
	MATRIX_3X4 = 16,
	MATRIX_4X2 = 17,
	MATRIX_4X3 = 18,
	MATRIX_4X4 = 19,
	INT1 = 20,
	INT2 = 21,
	INT3 = 22,
	INT4 = 23,
	BOOL = 24,
	STRUCT = 25,
	COLOR = 26,
	COUNT = 27,
	UNKNOWN = 0xffff
};

// note:不完善但基本够用
enum RHIParamObjectType
{
	SAMPLER1D = 1,
	SAMPLER2D = 2,
	SAMPLER3D = 3,
	SAMPLERCUBE = 4,
	TEXTURE1D = 11,
	TEXTURE2D = 12,
	TEXTURE3D = 13,
	TEXTURECUBE = 14,
	TEXTURE1DARRAY = 21,
	TEXTURE2DARRAY = 22,
	TEXTURE3DARRAY = 23,
	TEXTURECUBEARRAY = 24,
	UNKNOWN = 0xffff
};

// note:
// 在vulkan的shader中不能直接使用单一数据,只能使用uniform buffer(以下使用ParamBlock表示)
struct RHIParamDataInfo
{
	std::string name;
	RHIParamDataType type;
	uint32_t elementSize;
	uint32_t arraySize;
	uint32_t arrayElementStride;
	uint32_t paramBlockSlot;
	uint32_t paramBlockSet;
	//shader里面的数据会自动对齐(除非显示表达),所以内存的偏移不能直接按照数据类型去进行计算
	uint32_t memOffset;
};

struct RHIParamObjectInfo
{
	std::string name;
	RHIParamObjectType type;
	uint32_t slot;
	uint32_t set;
};

struct RHIParamBlockInfo
{
	std::string name;
	uint32_t slot;
	uint32_t set;
	uint32_t blockSize;
	//是否能让不同管线阶段共享
	bool isShareable = false;
};

// note:以后需要与glslang对接,所以结构类型可能稍作变更
struct RHIParamInfo
{
	std::map<std::string, RHIParamBlockInfo> paramBlocks;
	std::map<std::string, RHIParamDataInfo> params;
	std::map<std::string, RHIParamObjectInfo> samplers;
};

// note:目前只支持两种类型的着色器
struct RHIProgramParamListInfo
{
	RHIParamInfo vertexParams;
	RHIParamInfo fragmentParams;
};

class RHIProgramParam
{
public:
	RHIProgramParam();
	~RHIProgramParam();

private:

};

class RHIProgramParamList
{
public:
	RHIProgramParamList();
	~RHIProgramParamList();
	
private:

};

#endif
