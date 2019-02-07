#ifndef PROGRAM_REFLECTION_H
#define PROGRAM_REFLECTION_H
//SPIRV-Cross
//set only one!

enum ShaderDataType
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
struct UniformBufferBlockMember
{
	ShaderDataType type;
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
		
	}
public:
	std::vector<UniformBufferBlock> mBlocks;
	std::vector<UniformSampler2D> mSampler2Ds;
};
#endif
