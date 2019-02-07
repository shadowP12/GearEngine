#ifndef PROGRAM_H
#define PROGRAM_H
#include "ProgramVariation.h"
#include "ProgramReflection.h"

struct VariationData
{
	std::shared_ptr<ProgramVariation> variation;
	std::shared_ptr<ProgramReflection> reflection;
};

class Program
{
public:
	Program() {}
	~Program() {}
	virtual void addSource(std::string stage, std::string path);
	virtual VariationData& getProgram(const DefineList& dl, bool &hasProgram);
protected:
	VariationData createProgram(const DefineList& dl);
protected:
	std::map<const DefineList, VariationData> mProgramVariations;
	std::map<std::string, std::string> mSources;
};

#endif
