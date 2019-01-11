#include "ShaderTool.h"

std::vector<uint32_t> read_spirv_file(const char *path)
{
	FILE *file = fopen(path, "rb");
	if (!file)
	{
		fprintf(stderr, "Failed to open SPIR-V file: %s\n", path);
		return {};
	}

	fseek(file, 0, SEEK_END);
	long len = ftell(file) / sizeof(uint32_t);
	rewind(file);

	std::vector<uint32_t> spirv(len);
	if (fread(spirv.data(), sizeof(uint32_t), len, file) != size_t(len))
		spirv.clear();

	fclose(file);
	return spirv;
}

BaseDataType parseType(const spirv_cross::SPIRType & row_type)
{
	BaseDataType type;
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

uint32_t parseTypeSize(const spirv_cross::SPIRType & row_type)
{
	uint32_t size = 0;
	switch (row_type.basetype)
	{
	case spirv_cross::SPIRType::BaseType::Boolean:
		size = 4;
		break;
	case spirv_cross::SPIRType::BaseType::Int:
		size = 4;
		break;
	case spirv_cross::SPIRType::BaseType::Half:
		size = 4;
		break;
	case spirv_cross::SPIRType::BaseType::Float:
		size = 4;
		break;
	case spirv_cross::SPIRType::BaseType::Double:
		size = 8;
		break;
	default:
		size = 0;
		break;
	}
	if (row_type.basetype == spirv_cross::SPIRType::BaseType::Int)
	{
		if (row_type.array.size() > 0)
		{
			size = 4 * row_type.array[0];
		}
	}
	if (row_type.basetype == spirv_cross::SPIRType::BaseType::Float)
	{
		if (row_type.array.size() > 0)
		{
			size = 4 * row_type.array[0];
		}
		if (row_type.vecsize == 2 && row_type.columns == 1)
		{
			size = 8;
		}
		if (row_type.vecsize == 3 && row_type.columns == 1)
		{
			size = 12;
		}
		if (row_type.vecsize == 4 && row_type.columns == 1)
		{
			size = 16;
		}
		if (row_type.vecsize == 3 && row_type.columns == 3)
		{
			size = 36;
		}
		if (row_type.vecsize == 4 && row_type.columns == 4)
		{
			size = 64;
		}
	}
	return size;
}

ProgramInfo * getProgramInfo(std::vector<uint32_t>& bin)
{
	spirv_cross::CompilerGLSL  glsl(bin);
	spirv_cross::ShaderResources resources = glsl.get_shader_resources();
	ProgramInfo * info = new ProgramInfo();

	//parseStageInputs
	uint32_t stage_inputs_size = 0;
	for (auto &resource : resources.stage_inputs)
	{
		StageInput si;
		si.name = glsl.get_name(resource.id);
		spirv_cross::SPIRType type = glsl.get_type(resource.type_id);
		si.type = parseType(type);
		si.size = parseTypeSize(type);
		si.offset = stage_inputs_size;
		stage_inputs_size += si.size;
		info->stageInputs.push_back(si);
	}
	info->stageInputsSize = stage_inputs_size;

	//
	uint32_t setCount = 0;
	//parseUniformBuffers
	for (auto &resource : resources.uniform_buffers)
	{
		BlockBuffer ub;
		ub.name = glsl.get_name(resource.id);;
		spirv_cross::SPIRType type = glsl.get_type(resource.base_type_id);

		uint32_t ub_size = 0;
		uint32_t member_count = type.member_types.size();
		for (int i = 0; i < member_count; i++)
		{
			BlockBufferMember ubm;
			ubm.name = glsl.get_member_name(resource.base_type_id, i);
			auto &member_type = glsl.get_type(type.member_types[i]);
			ubm.type = parseType(member_type);
			ubm.size = parseTypeSize(member_type);
			ubm.offset = ub_size;
			ub_size += ubm.size;
			ub.members.push_back(ubm);
		}
		ub.size = ub_size;
		ub.set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		ub.binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
		if (setCount < ub.set)
		{
			setCount = ub.set;
		}
		info->blockBuffers.push_back(ub);
	}

	//parseSampler2Ds
	for (auto &resource : resources.sampled_images)
	{
		Sampler2D s;
		s.name = glsl.get_name(resource.id);
		s.set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		s.binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
		if (setCount < s.set)
		{
			setCount = s.set;
		}
		info->sampler2Ds.push_back(s);
	}

	info->setCount = setCount;
	return info;
}
