#ifndef SPIRV_MANAGER_H
#define SPIRV_MANAGER_H
#include "../../Utility/Module.h"
#include <vector>

enum SpirvStageType
{
    STAGE_VERTEX,
    STAGE_FRAGMENT,
    STAGE_COMPUTE
};

struct SpirvCompileInfo
{
    std::string source;
    std::string entryPoint;
    std::string includeDir;
    SpirvStageType stageType = STAGE_VERTEX;
    //TODO: program define
};

struct SpirvCompileResult
{
    std::vector<uint32_t> bytes;
};

class SpirvManager : public Module<SpirvManager>
{
public:
    SpirvManager();
	~SpirvManager();
    SpirvCompileResult compile(const SpirvCompileInfo& info);
};

#endif