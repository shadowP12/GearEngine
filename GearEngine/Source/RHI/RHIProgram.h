#ifndef RHI_PROGRAM_H
#define RHI_PROGRAM_H
#include "RHIDefine.h"
#include <string>
#include <vector>

class RHIDevice;
class RHIProgramManager;
class RHIUniformBuffer;
class RHITexture;
class RHITextureView;

struct RHIProgramInfo
{
    std::vector<uint32_t> bytes;
    ProgramType type;
};

class RHIProgram
{
public:
	RHIProgram(RHIDevice* device, const RHIProgramInfo& info);
	~RHIProgram();
	VkShaderModule getHandle() const { return mModule; }
private:
	RHIDevice* mDevice;
	RHIProgramManager* mProgramMgr;
	VkShaderModule mModule;
    ProgramType mType;
};
#endif