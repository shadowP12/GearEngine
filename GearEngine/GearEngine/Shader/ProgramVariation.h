#ifndef PROGRAM_VARIATION_H
#define PROGRAM_VARIATION_H
#include "Shader.h"
#include "ProgramReflection.h"
#include <map>
#include <memory>

class DefineList : public std::map<std::string, std::string>
{
public:
	DefineList& add(const std::string& name, const std::string& val = "") { (*this)[name] = val; return *this; }
	DefineList& remove(const std::string& name) { (*this).erase(name); return *this; }
};

class ProgramVariation
{
public:
	ProgramVariation(std::shared_ptr<Shader> vs, std::shared_ptr<Shader> ps)
	{
		mVS = vs;
		mPS = ps;
	}
	~ProgramVariation() {}

private:
	std::shared_ptr<Shader> mVS;
	std::shared_ptr<Shader> mPS;
	//todo: more
};
#endif
