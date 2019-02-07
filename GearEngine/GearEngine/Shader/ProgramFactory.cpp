#include "ProgramFactory.h"
#include <glslang/public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <spirv_glsl.hpp>
#include <spirv_reflect.hpp>
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif
const TBuiltInResource DefaultTBuiltInResource = {
	/* .MaxLights = */ 32,
	/* .MaxClipPlanes = */ 6,
	/* .MaxTextureUnits = */ 32,
	/* .MaxTextureCoords = */ 32,
	/* .MaxVertexAttribs = */ 64,
	/* .MaxVertexUniformComponents = */ 4096,
	/* .MaxVaryingFloats = */ 64,
	/* .MaxVertexTextureImageUnits = */ 32,
	/* .MaxCombinedTextureImageUnits = */ 80,
	/* .MaxTextureImageUnits = */ 32,
	/* .MaxFragmentUniformComponents = */ 4096,
	/* .MaxDrawBuffers = */ 32,
	/* .MaxVertexUniformVectors = */ 128,
	/* .MaxVaryingVectors = */ 8,
	/* .MaxFragmentUniformVectors = */ 16,
	/* .MaxVertexOutputVectors = */ 16,
	/* .MaxFragmentInputVectors = */ 15,
	/* .MinProgramTexelOffset = */ -8,
	/* .MaxProgramTexelOffset = */ 7,
	/* .MaxClipDistances = */ 8,
	/* .MaxComputeWorkGroupCountX = */ 65535,
	/* .MaxComputeWorkGroupCountY = */ 65535,
	/* .MaxComputeWorkGroupCountZ = */ 65535,
	/* .MaxComputeWorkGroupSizeX = */ 1024,
	/* .MaxComputeWorkGroupSizeY = */ 1024,
	/* .MaxComputeWorkGroupSizeZ = */ 64,
	/* .MaxComputeUniformComponents = */ 1024,
	/* .MaxComputeTextureImageUnits = */ 16,
	/* .MaxComputeImageUniforms = */ 8,
	/* .MaxComputeAtomicCounters = */ 8,
	/* .MaxComputeAtomicCounterBuffers = */ 1,
	/* .MaxVaryingComponents = */ 60,
	/* .MaxVertexOutputComponents = */ 64,
	/* .MaxGeometryInputComponents = */ 64,
	/* .MaxGeometryOutputComponents = */ 128,
	/* .MaxFragmentInputComponents = */ 128,
	/* .MaxImageUnits = */ 8,
	/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
	/* .MaxCombinedShaderOutputResources = */ 8,
	/* .MaxImageSamples = */ 0,
	/* .MaxVertexImageUniforms = */ 0,
	/* .MaxTessControlImageUniforms = */ 0,
	/* .MaxTessEvaluationImageUniforms = */ 0,
	/* .MaxGeometryImageUniforms = */ 0,
	/* .MaxFragmentImageUniforms = */ 8,
	/* .MaxCombinedImageUniforms = */ 8,
	/* .MaxGeometryTextureImageUnits = */ 16,
	/* .MaxGeometryOutputVertices = */ 256,
	/* .MaxGeometryTotalOutputComponents = */ 1024,
	/* .MaxGeometryUniformComponents = */ 1024,
	/* .MaxGeometryVaryingComponents = */ 64,
	/* .MaxTessControlInputComponents = */ 128,
	/* .MaxTessControlOutputComponents = */ 128,
	/* .MaxTessControlTextureImageUnits = */ 16,
	/* .MaxTessControlUniformComponents = */ 1024,
	/* .MaxTessControlTotalOutputComponents = */ 4096,
	/* .MaxTessEvaluationInputComponents = */ 128,
	/* .MaxTessEvaluationOutputComponents = */ 128,
	/* .MaxTessEvaluationTextureImageUnits = */ 16,
	/* .MaxTessEvaluationUniformComponents = */ 1024,
	/* .MaxTessPatchComponents = */ 120,
	/* .MaxPatchVertices = */ 32,
	/* .MaxTessGenLevel = */ 64,
	/* .MaxViewports = */ 16,
	/* .MaxVertexAtomicCounters = */ 0,
	/* .MaxTessControlAtomicCounters = */ 0,
	/* .MaxTessEvaluationAtomicCounters = */ 0,
	/* .MaxGeometryAtomicCounters = */ 0,
	/* .MaxFragmentAtomicCounters = */ 8,
	/* .MaxCombinedAtomicCounters = */ 8,
	/* .MaxAtomicCounterBindings = */ 1,
	/* .MaxVertexAtomicCounterBuffers = */ 0,
	/* .MaxTessControlAtomicCounterBuffers = */ 0,
	/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
	/* .MaxGeometryAtomicCounterBuffers = */ 0,
	/* .MaxFragmentAtomicCounterBuffers = */ 1,
	/* .MaxCombinedAtomicCounterBuffers = */ 1,
	/* .MaxAtomicCounterBufferSize = */ 16384,
	/* .MaxTransformFeedbackBuffers = */ 4,
	/* .MaxTransformFeedbackInterleavedComponents = */ 64,
	/* .MaxCullDistances = */ 8,
	/* .MaxCombinedClipAndCullDistances = */ 8,
	/* .MaxSamples = */ 4,
	/* .maxMeshOutputVerticesNV = */ 256,
	/* .maxMeshOutputPrimitivesNV = */ 512,
	/* .maxMeshWorkGroupSizeX_NV = */ 32,
	/* .maxMeshWorkGroupSizeY_NV = */ 1,
	/* .maxMeshWorkGroupSizeZ_NV = */ 1,
	/* .maxTaskWorkGroupSizeX_NV = */ 32,
	/* .maxTaskWorkGroupSizeY_NV = */ 1,
	/* .maxTaskWorkGroupSizeZ_NV = */ 1,
	/* .maxMeshViewCountNV = */ 4,

	/* .limits = */ {
	/* .nonInductiveForLoops = */ 1,
	/* .whileLoops = */ 1,
	/* .doWhileLoops = */ 1,
	/* .generalUniformIndexing = */ 1,
	/* .generalAttributeMatrixVectorIndexing = */ 1,
	/* .generalVaryingIndexing = */ 1,
	/* .generalSamplerIndexing = */ 1,
	/* .generalVariableIndexing = */ 1,
	/* .generalConstantMatrixVectorIndexing = */ 1,
} };
EShLanguage GetShaderStage(const std::string& stage)
{
	if (stage == "vert")
	{
		return EShLangVertex;
	}
	else if (stage == "tesc")
	{
		return EShLangTessControl;
	}
	else if (stage == "tese")
	{
		return EShLangTessEvaluation;
	}
	else if (stage == "geom")
	{
		return EShLangGeometry;
	}
	else if (stage == "frag")
	{
		return EShLangFragment;
	}
	else if (stage == "comp")
	{
		return EShLangCompute;
	}
	else
	{
		assert(0 && "Unknown shader stage");
		return EShLangCount;
	}
}
ShaderDataType parseType(const spirv_cross::SPIRType & row_type)
{
	ShaderDataType type;
	switch (row_type.basetype)
	{
	case spirv_cross::SPIRType::BaseType::Boolean:
		type = Boolean;
		break;
	case spirv_cross::SPIRType::BaseType::Int:
		type = Int;
		break;
	case spirv_cross::SPIRType::BaseType::Half:
		type = Half;
		break;
	case spirv_cross::SPIRType::BaseType::Float:
		type = Float;
		break;
	case spirv_cross::SPIRType::BaseType::Double:
		type = Double;
		break;
	default:
		type = None;
		break;
	}
	if (type == Int)
	{
		if (row_type.array.size() > 0)
		{
			type = IntArray;
		}
	}
	if (type == Float)
	{
		if (row_type.array.size() > 0)
		{
			type = FloatArray;
		}
		if (row_type.vecsize == 2 && row_type.columns == 1)
		{
			type = Vector2;
		}
		if (row_type.vecsize == 3 && row_type.columns == 1)
		{
			type = Vector3;
		}
		if (row_type.vecsize == 4 && row_type.columns == 1)
		{
			type = Vector4;
		}
		if (row_type.vecsize == 3 && row_type.columns == 3)
		{
			type = Matrix3;
		}
		if (row_type.vecsize == 4 && row_type.columns == 4)
		{
			type = Matrix4;
		}
	}
	return type;
}
//parse shader
std::shared_ptr<ProgramReflection> createReflection(std::vector<uint32_t> &spirv)
{
	spirv_cross::CompilerGLSL  glsl(spirv);
	spirv_cross::ShaderResources resources = glsl.get_shader_resources();

	//parse uniformBuffers
	std::vector<UniformBufferBlock> blocks;
	for (auto &resource : resources.uniform_buffers)
	{
		UniformBufferBlock ub;
		ub.name = glsl.get_name(resource.id);;
		spirv_cross::SPIRType type = glsl.get_type(resource.base_type_id);

		uint32_t ub_size = 0;
		uint32_t member_count = type.member_types.size();
		for (int i = 0; i < member_count; i++)
		{
			UniformBufferBlockMember ubm;
			ubm.name = glsl.get_member_name(resource.base_type_id, i);
			auto &member_type = glsl.get_type(type.member_types[i]);
			ubm.type = parseType(member_type);

			ubm.size = glsl.get_declared_struct_member_size(type, i);
			ubm.offset = glsl.type_struct_member_offset(type, i);

			ub_size += ubm.size;
			ub.members.push_back(ubm);
		}
		ub.size = ub_size;
		ub.set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		ub.binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

		blocks.push_back(ub);
	}
	//parse sampler2Ds
	std::vector<UniformSampler2D> sampler2Ds;
	for (auto &resource : resources.sampled_images)
	{
		UniformSampler2D s;
		s.name = glsl.get_name(resource.id);
		s.set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		s.binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

		sampler2Ds.push_back(s);
	}

	std::shared_ptr<ProgramReflection> res = std::shared_ptr<ProgramReflection>(new ProgramReflection());
	return res;
}

ProgramFactory::ProgramFactory()
{
	glslang::InitializeProcess();
}

ProgramFactory::~ProgramFactory()
{
	glslang::FinalizeProcess();
}

std::shared_ptr<Shader> ProgramFactory::createShader(const std::string & stage, const std::string & source, std::shared_ptr<ProgramReflection>& reflection)
{
	EShLanguage shaderType = GetShaderStage(stage);
	glslang::TShader shader(shaderType);
	const char* inputSource = source.c_str();
	shader.setStrings(&inputSource, 1);

	EShMessages messages = (EShMessages)((int)EShMsgSpvRules | (int)EShMsgVulkanRules);
	TBuiltInResource resources = DefaultTBuiltInResource;
	if (!shader.parse(&resources, 450, false, messages))
	{
		throw std::runtime_error("failed to parse shader!");
	}
	glslang::TProgram program;
	program.addShader(&shader);

	if (!program.link(messages))
	{
		throw std::runtime_error("failed to link program!");
	}

	program.mapIO();

	std::vector<unsigned int> spirv;
	spv::SpvBuildLogger logger;

	GlslangToSpv(*program.getIntermediate(shaderType), spirv, &logger);

	std::shared_ptr<Shader> res = std::shared_ptr<Shader>(new Shader(spirv));
	return res;
}
