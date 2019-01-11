#ifndef SHADER_INFO_H
#define SHADER_INFO_H
#include <vector>
#include <map>
#include <string>
enum BaseDataType
{
	None,
	Boolean,
	Int,
	Half,
	Float,
	Double,
	IntArray,
	FloatArray,
	Vector2,
	Vector3,
	Vector4,
	Matrix3,
	Matrix4
};
struct SingleData
{
	BaseDataType type;
	uint32_t size;
};

struct BlockBufferMember
{
	BaseDataType type;
	std::string name;
	uint32_t size;
	uint32_t offset;
};

struct BlockBuffer
{
	std::string name;
	std::vector<BlockBufferMember> members;
	uint32_t size;
	uint32_t set;
	uint32_t binding;
};

struct Sampler2D
{
	std::string name;
	uint32_t set;
	uint32_t binding;
};
struct StageInput
{
	BaseDataType type;
	std::string name;
	uint32_t size;
	uint32_t offset;
};

struct ProgramInfo
{
	std::vector<BlockBuffer> blockBuffers;
	std::vector<StageInput> stageInputs;
	std::vector<Sampler2D> sampler2Ds;
	uint32_t stageInputsSize;
	uint32_t setCount;
};
#endif