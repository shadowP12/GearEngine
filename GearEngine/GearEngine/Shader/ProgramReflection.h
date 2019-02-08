#ifndef PROGRAM_REFLECTION_H
#define PROGRAM_REFLECTION_H
//SPIRV-Cross
#include <vector>
enum ReflectionDataType
{
	NONE = 1,
	B,
	I,
	H,
	F,
	D,
	IA,
	FA,
	VEC2,
	VEC3,
	VEC4,
	MAT3,
	MAT4
};
struct UniformBufferBlockMember
{
	ReflectionDataType type;
	std::string name;
	uint32_t size;
	uint32_t offset;
};
struct UniformBufferBlock
{
	std::string name;
	std::vector<UniformBufferBlockMember> members;
	uint32_t size;
	uint32_t set;//set = 1
	uint32_t binding;
};
struct UniformSampler2D
{
	std::string name;
	uint32_t set;//set = 1
	uint32_t binding;
};

class ProgramReflection : public std::enable_shared_from_this<ProgramReflection>
{
public:
	ProgramReflection() {}

	~ProgramReflection() {}

	std::vector<UniformBufferBlock>& getBlocks() { return mBlocks; }

	void setBlocks(std::vector<UniformBufferBlock> blocks)
	{
		mBlocks = blocks;
	}

	std::vector<UniformSampler2D>& getSampler2Ds() { return mSampler2Ds; }

	void setSampler2Ds(std::vector<UniformSampler2D> sampler2Ds)
	{
		mSampler2Ds = sampler2Ds;
	}

	void addBlocks(std::vector<UniformBufferBlock> blocks)
	{
		for (uint32_t i = 0; i < blocks.size(); i++)
		{
			mBlocks.push_back(blocks[i]);
		}
	}

	void addSampler2Ds(std::vector<UniformSampler2D> sampler2Ds)
	{
		for (uint32_t i = 0; i < sampler2Ds.size(); i++)
		{
			mSampler2Ds.push_back(sampler2Ds[i]);
		}
	}
public:
	std::vector<UniformBufferBlock> mBlocks;
	std::vector<UniformSampler2D> mSampler2Ds;
};
#endif
