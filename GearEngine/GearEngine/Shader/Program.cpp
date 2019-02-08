#include "Program.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "ProgramFactory.h"
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
	VariationData res;
	const auto& it = mProgramVariations.find(dl);
	if (it == mProgramVariations.end())
	{
		res = createProgram(dl);
	}
	else
	{
		res = mProgramVariations[dl];
	}
	return res;
}

VariationData Program::createProgram(const DefineList & dl)
{
	VariationData res;

	std::stringstream defineBlock;

	for (const auto &define : dl)
	{
		defineBlock << "#define " << define.first << " " << define.second << "\n";
	}
	std::shared_ptr<Shader> vs;
	std::shared_ptr<Shader> ps;
	std::shared_ptr<ProgramReflection> reflection = std::shared_ptr<ProgramReflection>(new ProgramReflection());
	for (auto source : mSources)
	{
		std::stringstream newSource;
		newSource << source.second;
		std::vector<UniformBufferBlock> blocks;
		std::vector<UniformSampler2D> sampler2ds;
		if(source.first == "vert")
			vs = ProgramFactory::instance().createShader(source.first, newSource.str(), blocks, sampler2ds);
		else if (source.first == "frag")
			ps = ProgramFactory::instance().createShader(source.first, newSource.str(), blocks, sampler2ds);

		reflection->addBlocks(blocks);
		reflection->addSampler2Ds(sampler2ds);
	}
	std::shared_ptr<ProgramVariation> variation = std::shared_ptr<ProgramVariation>(new ProgramVariation(vs, ps));

	res.reflection = reflection;
	res.variation = variation;
	return res;
}


