#include "SpirvManager.h"
#include "glslang/Include/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"
#include "spirv.hpp"
#include "spirv_glsl.hpp"
#include "spirv_reflect.hpp"
#include "StandAlone/DirStackFileIncluder.h"
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

SpirvManager::SpirvManager()
{
	glslang::InitializeProcess();
}

SpirvManager::~SpirvManager()
{
	glslang::FinalizeProcess();
}

SpirvCompileResult SpirvManager::compile(const SpirvCompileInfo& info)
{
    SpirvCompileResult res;
    res.bytes.clear();

	EShLanguage glslType;
	switch (info.stageType)
	{
	case STAGE_VERTEX:
		glslType = EShLangVertex;
		break;
	case STAGE_FRAGMENT:
	    glslType = EShLangFragment;
	    break;
	case STAGE_COMPUTE:
	    glslType = EShLangCompute;
	    break;
	default:
		break;
	}

	glslang::TShader shader(glslType);
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

	const char* sourceBytes = info.source.c_str();
	shader.setStrings(&sourceBytes, 1);
	shader.setEntryPoint("main");
	//shader.setPreamble(preamble.c_str());

    DirStackFileIncluder includer;
    includer.pushExternalLocalDirectory(info.includeDir);

	TBuiltInResource resources = DefaultTBuiltInResource;
	EShMessages messages = (EShMessages)((int)EShMsgSpvRules | (int)EShMsgVulkanRules);
	if (!shader.parse(&resources, 450, false, messages, includer))
	{
		LOGE("%s\n", shader.getInfoLog());
		return res;
	}

	glslang::TProgram program;
	program.addShader(&shader);

	if (!program.link(messages))
	{
		LOGE("%s\n", program.getInfoLog());
		return res;
	}

	program.mapIO();
	program.buildReflection(EShReflectionAllBlockVariables);

	spv::SpvBuildLogger logger;
	glslang::SpvOptions options;
	options.disableOptimizer = true;
	options.optimizeSize = true;
	glslang::GlslangToSpv(*program.getIntermediate(glslType), res.bytes, &options);
	return res;
}
