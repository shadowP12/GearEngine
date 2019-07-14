#ifndef RHI_PROGRAM_H
#define RHI_PROGRAM_H
#include <string>

enum class RHIProgramType
{
	Vertex,
	Fragment
};

struct RHIProgramDesc
{
	std::string source;
	std::string entryPoint;
	RHIProgramType type = RHIProgramType::Vertex;
};

//还需要Program参数以及顶点数据输入布局
#endif