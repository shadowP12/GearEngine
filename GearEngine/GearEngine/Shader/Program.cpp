#include "Program.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

void Program::addSource(std::string stage, std::string path)
{
	std::ifstream file(path);

	if (!file.is_open())
	{
		std::cout << "Failed to load shader: " << path.c_str() << std::endl;
		throw std::runtime_error("failed to open file: " + path);
	}

	std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	mSources[stage] = source;
}

VariationData & Program::getProgram(const DefineList & dl, bool & hasProgram)
{

}

VariationData Program::createProgram(const DefineList & dl)
{
	VariationData res;

	std::stringstream defineBlock;

	for (const auto &define : dl)
	{
		defineBlock << "#define " << define.first << " " << define.second << "\n";
	}

	
	for (auto source : mSources)
	{
		
	}
	return res;
}


