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
	//program define
};

class RHIProgram
{
public:
	RHIProgram();
	~RHIProgram();

private:

};
#endif