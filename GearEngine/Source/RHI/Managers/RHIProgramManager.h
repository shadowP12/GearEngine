#ifndef RHI_PROGRAM_MANAGER_H
#define RHI_PROGRAM_MANAGER_H
#include "RHI/RHIProgram.h"
#include "Utility/Module.h"
/**
   负责program的实时编译
*/
class RHIProgramManager : public Module<RHIProgramManager>
{
public:
	RHIProgramManager();
	~RHIProgramManager();
	void compile(RHIProgram* program);
private:
};

#endif