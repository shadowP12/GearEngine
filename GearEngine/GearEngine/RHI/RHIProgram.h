#ifndef RHI_PROGRAM_H
#define RHI_PROGRAM_H
#include <string>
#include "RHIProgramParam.h"
enum class RHIProgramType
{
	Vertex,
	Fragment
};

struct RHIProgramInfo
{
	std::string source;
	std::string entryPoint;
	RHIProgramType type = RHIProgramType::Vertex;
	//TODO: program define
	//TODO: program include
};

class RHIDevice;

class RHIProgram
{
public:
	RHIProgram(RHIDevice* device, const RHIProgramInfo& info);
	~RHIProgram();
	void compile();
private:
	friend class RHIProgramManager;
	RHIDevice* mDevice;
	VkShaderModule mModule;
	std::string mSource;
	std::string mEntryPoint;
	RHIProgramType mType;
	//编译后的产物
	std::vector<unsigned int> mBytecode;
	RHIParamInfo mParamInfo;
};
#endif