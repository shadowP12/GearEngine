#ifndef RHI_PROGRAM_MANAGER_H
#define RHI_PROGRAM_MANAGER_H

#include "../RHIProgram.h"
#include <map>

class RHIDevice;

class RHIProgramManager
{
public:
	RHIProgramManager(RHIDevice* device);
	~RHIProgramManager();
    RHIProgram* createProgram(const RHIProgramInfo& info);
    void deleteProgram(RHIProgram* program);
	void compile(RHIProgram* program);
private:
    friend class RHIProgram;
    RHIDevice* mDevice;
    std::map<uint32_t, RHIProgram*> mPrograms;
    static uint32_t sNextValidID;
};

#endif