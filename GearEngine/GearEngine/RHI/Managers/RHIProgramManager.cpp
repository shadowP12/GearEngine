#include "RHIProgramManager.h"
#include "glslang/Include/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"
#include "Utility/Log.h"

const TBuiltInResource DefaultTBuiltInResource = {
	/*.maxLights = */ 8,         // From OpenGL 3.0 table 6.46.
	/*.maxClipPlanes = */ 6,     // From OpenGL 3.0 table 6.46.
	/*.maxTextureUnits = */ 2,   // From OpenGL 3.0 table 6.50.
	/*.maxTextureCoords = */ 8,  // From OpenGL 3.0 table 6.50.
	/*.maxVertexAttribs = */ 16,
	/*.maxVertexUniformComponents = */ 4096,
	/*.maxVaryingFloats = */ 60,  // From OpenGLES 3.1 table 6.44.
	/*.maxVertexTextureImageUnits = */ 16,
	/*.maxCombinedTextureImageUnits = */ 80,
	/*.maxTextureImageUnits = */ 16,
	/*.maxFragmentUniformComponents = */ 1024,

	// glslang has 32 maxDrawBuffers.
	// Pixel phone Vulkan driver in Android N has 8
	// maxFragmentOutputAttachments.
	/*.maxDrawBuffers = */ 8,

	/*.maxVertexUniformVectors = */ 256,
	/*.maxVaryingVectors = */ 15,  // From OpenGLES 3.1 table 6.44.
	/*.maxFragmentUniformVectors = */ 256,
	/*.maxVertexOutputVectors = */ 16,   // maxVertexOutputComponents / 4
	/*.maxFragmentInputVectors = */ 15,  // maxFragmentInputComponents / 4
	/*.minProgramTexelOffset = */ -8,
	/*.maxProgramTexelOffset = */ 7,
	/*.maxClipDistances = */ 8,
	/*.maxComputeWorkGroupCountX = */ 65535,
	/*.maxComputeWorkGroupCountY = */ 65535,
	/*.maxComputeWorkGroupCountZ = */ 65535,
	/*.maxComputeWorkGroupSizeX = */ 1024,
	/*.maxComputeWorkGroupSizeX = */ 1024,
	/*.maxComputeWorkGroupSizeZ = */ 64,
	/*.maxComputeUniformComponents = */ 512,
	/*.maxComputeTextureImageUnits = */ 16,
	/*.maxComputeImageUniforms = */ 8,
	/*.maxComputeAtomicCounters = */ 8,
	/*.maxComputeAtomicCounterBuffers = */ 1,  // From OpenGLES 3.1 Table 6.43
	/*.maxVaryingComponents = */ 60,
	/*.maxVertexOutputComponents = */ 64,
	/*.maxGeometryInputComponents = */ 64,
	/*.maxGeometryOutputComponents = */ 128,
	/*.maxFragmentInputComponents = */ 128,
	/*.maxImageUnits = */ 8,  // This does not seem to be defined anywhere,
	// set to ImageUnits.
	/*.maxCombinedImageUnitsAndFragmentOutputs = */ 8,
	/*.maxCombinedShaderOutputResources = */ 8,
	/*.maxImageSamples = */ 0,
	/*.maxVertexImageUniforms = */ 0,
	/*.maxTessControlImageUniforms = */ 0,
	/*.maxTessEvaluationImageUniforms = */ 0,
	/*.maxGeometryImageUniforms = */ 0,
	/*.maxFragmentImageUniforms = */ 8,
	/*.maxCombinedImageUniforms = */ 8,
	/*.maxGeometryTextureImageUnits = */ 16,
	/*.maxGeometryOutputVertices = */ 256,
	/*.maxGeometryTotalOutputComponents = */ 1024,
	/*.maxGeometryUniformComponents = */ 512,
	/*.maxGeometryVaryingComponents = */ 60,  // Does not seem to be defined
	// anywhere, set equal to
	// maxVaryingComponents.
	/*.maxTessControlInputComponents = */ 128,
	/*.maxTessControlOutputComponents = */ 128,
	/*.maxTessControlTextureImageUnits = */ 16,
	/*.maxTessControlUniformComponents = */ 1024,
	/*.maxTessControlTotalOutputComponents = */ 4096,
	/*.maxTessEvaluationInputComponents = */ 128,
	/*.maxTessEvaluationOutputComponents = */ 128,
	/*.maxTessEvaluationTextureImageUnits = */ 16,
	/*.maxTessEvaluationUniformComponents = */ 1024,
	/*.maxTessPatchComponents = */ 120,
	/*.maxPatchVertices = */ 32,
	/*.maxTessGenLevel = */ 64,
	/*.maxViewports = */ 16,
	/*.maxVertexAtomicCounters = */ 0,
	/*.maxTessControlAtomicCounters = */ 0,
	/*.maxTessEvaluationAtomicCounters = */ 0,
	/*.maxGeometryAtomicCounters = */ 0,
	/*.maxFragmentAtomicCounters = */ 8,
	/*.maxCombinedAtomicCounters = */ 8,
	/*.maxAtomicCounterBindings = */ 1,
	/*.maxVertexAtomicCounterBuffers = */ 0,  // From OpenGLES 3.1 Table 6.41.
	// ARB_shader_atomic_counters.
	/*.maxTessControlAtomicCounterBuffers = */ 0,
	/*.maxTessEvaluationAtomicCounterBuffers = */ 0,
	/*.maxGeometryAtomicCounterBuffers = */ 0,  // /ARB_shader_atomic_counters.
	/*.maxFragmentAtomicCounterBuffers = */ 0,  // From OpenGLES 3.1 Table 6.43.
	/*.maxCombinedAtomicCounterBuffers = */ 1,
	/*.maxAtomicCounterBufferSize = */ 32,
	/*.maxTransformFeedbackBuffers = */ 4,
	/*.maxTransformFeedbackInterleavedComponents = */ 64,
	/*.maxCullDistances = */ 8,  // ARB_cull_distance.
	/*.maxCombinedClipAndCullDistances = */ 8,  // ARB_cull_distance.
	/*.maxSamples = */ 4,
	/* .maxMeshOutputVerticesNV = */ 256,
	/* .maxMeshOutputPrimitivesNV = */ 512,
	/* .maxMeshWorkGroupSizeX_NV = */ 32,
	/* .maxMeshWorkGroupSizeY_NV = */ 1,
	/* .maxMeshWorkGroupSizeZ_NV = */ 1,
	/* .maxTaskWorkGroupSizeX_NV = */ 32,
	/* .maxTaskWorkGroupSizeY_NV = */ 1,
	/* .maxTaskWorkGroupSizeZ_NV = */ 1,
	/* .maxMeshViewCountNV = */ 4,  // This is the glslang TLimits structure.
									// It defines whether or not the following features are enabled.
									// We want them to all be enabled.
	/*.limits = */ {
	/*.nonInductiveForLoops = */ 1,
	/*.whileLoops = */ 1,
	/*.doWhileLoops = */ 1,
	/*.generalUniformIndexing = */ 1,
	/*.generalAttributeMatrixVectorIndexing = */ 1,
	/*.generalVaryingIndexing = */ 1,
	/*.generalSamplerIndexing = */ 1,
	/*.generalVariableIndexing = */ 1,
	/*.generalConstantMatrixVectorIndexing = */ 1,
} };

RHIProgramManager::RHIProgramManager()
{
	glslang::InitializeProcess();
}

RHIProgramManager::~RHIProgramManager()
{
	glslang::FinalizeProcess();
}

void RHIProgramManager::compile(RHIProgram* rhiProgram)
{
	EShLanguage glslType = EShLangVertex;

	EShLanguage glslType;
	switch (rhiProgram->mType)
	{
	case RHIProgramType::Fragment:
		glslType = EShLangFragment;
		break;
	case RHIProgramType::Vertex:
		glslType = EShLangVertex;
		break;
	default:
		break;
	}

	glslang::TShader shader(glslType);
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);

	const char* sourceBytes = rhiProgram->mSource.c_str();
	shader.setStrings(&sourceBytes, 1);

	//暂时设置为"main"
	shader.setEntryPoint("main");
	//预处理此版本不加入
	//shader.setPreamble(preamble.c_str());

	TBuiltInResource resources = DefaultTBuiltInResource;
	EShMessages messages = (EShMessages)((int)EShMsgSpvRules | (int)EShMsgVulkanRules);
	if (!shader.parse(&resources, 450, false, messages))
	{
		LOGE("%s\n", shader.getInfoLog());
		return;
	}

	glslang::TProgram program;
	program.addShader(&shader);

	if (!program.link(messages))
	{
		LOGE("%s\n", program.getInfoLog());
		return;
	}

	program.mapIO();
	program.buildReflection(EShReflectionAllBlockVariables);

	spv::SpvBuildLogger logger;
	glslang::SpvOptions options;
	options.disableOptimizer = true;
	options.optimizeSize = false;
	glslang::GlslangToSpv(*program.getIntermediate(glslType), rhiProgram->mBytecode, &options);
	
	//有反射信息去构建shader里面的参数布局
	int numUniformBlocks = program.getNumUniformBlocks();
	for (int i = 0; i < numUniformBlocks; i++)
	{
		const glslang::TObjectReflection& uniformBlock = program.getUniformBlock(i);
		RHIParamBlockInfo blockInfo;;
		blockInfo.name = uniformBlock.name;
		blockInfo.blockSize = uniformBlock.size;
		blockInfo.set = uniformBlock.getType()->getQualifier().layoutSet;
		blockInfo.slot = uniformBlock.getType()->getQualifier().layoutBinding;

		rhiProgram->mParamInfo.paramBlocks[uniformBlock.name] = blockInfo;
	}

	int numUniformVariables = program.getNumUniformVariables();
	for (int i = 0; i < numUniformVariables; i++)
	{
		//暂时只支持几种基本类型
		const glslang::TObjectReflection& uniformVar = program.getUniform(i);
		const glslang::TType* ttype = program.getUniformTType(i);
		const glslang::TQualifier& qualifier = ttype->getQualifier();
		if (ttype->getBasicType() == glslang::EbtSampler)
		{
			RHIParamObjectInfo objInfo;
			objInfo.name = uniformVar.name;
			//需要添加各种采样器类型判断
			objInfo.type = RHIParamObjectType::SAMPLER2D;
			objInfo.set = qualifier.layoutSet;
			objInfo.slot = qualifier.layoutBinding;
			rhiProgram->mParamInfo.samplers[uniformVar.name] = objInfo;
			continue;
		}
		if (ttype->getBasicType() == glslang::EbtStruct)
		{
			//暂时不支持结构体
			continue;
		}
		if (ttype->getBasicType() == glslang::EbtBlock)
		{
			//暂时不支持数据块
			continue;
		}
		//暂时不支持数组以及类型判断(...大版本稳定后会加上)
		RHIParamDataInfo dataInfo;
		dataInfo.name = uniformVar.name;
		dataInfo.memOffset = uniformVar.offset;
		const glslang::TObjectReflection& uniformBlock = program.getUniformBlock(uniformVar.index);
		dataInfo.paramBlockSet = uniformBlock.getType()->getQualifier().layoutSet;
		dataInfo.paramBlockSlot = uniformBlock.getType()->getQualifier().layoutBinding;
		
		rhiProgram->mParamInfo.params[uniformVar.name] = dataInfo;
	}
}
