#ifndef PROGRAM_FACTORY_H
#define PROGRAM_FACTORY_H
#include "../Utility/Module.h"
#include "ProgramReflection.h"
#include "Shader.h"
class ProgramFactory : public Module<ProgramFactory>
{
public:
	ProgramFactory();
	~ProgramFactory();
	std::shared_ptr<Shader> createShader(const std::string& stage, const std::string& source, std::shared_ptr<ProgramReflection>& reflection);
private:

};

#endif
