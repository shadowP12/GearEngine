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
	//TODO: program define
	//TODO: program include
};

class RHIDevice;
class RHIProgramManager;
class RHIUniformBuffer;
class RHITexture;
class RHITextureView;

class RHIProgram : public RHIObject
{
public:
	RHIProgram(RHIDevice* device, RHIProgramManager* mgr, const RHIProgramInfo& info);
	~RHIProgram();
	void compile();
	VkShaderModule getHandle() const { return mModule; }
    void setUniformBuffer(std::string name, RHIUniformBuffer* ub);
private:
	friend class RHIProgramManager;
	friend class RHIGraphicsPipelineState;
	RHIDevice* mDevice;
	RHIProgramManager* mProgramMgr;
	VkShaderModule mModule;
    std::map<uint32_t, VkDescriptorSet> mDescriptorSets;
	std::string mSource;
	std::string mEntryPoint;
	RHIProgramType mType;
	//编译后的产物
	std::vector<uint32_t> mBytecode;
	std::vector<RHIProgramParameter::UniformBuffer> mUniformBufferInfos;
    std::vector<RHIProgramParameter::SampleImage> mSampleImageInfos;
	RHIParamInfo mParamInfo;
	bool mIsCompiled = false;
};
#endif