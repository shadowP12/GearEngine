#ifndef SHADER_TOOL_H
#define SHADER_TOOL_H
#include "ShaderInfo.h"
#include "spirv_glsl.hpp"
#include "spirv_reflect.hpp"
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif
std::vector<uint32_t> read_spirv_file(const char *path);
BaseDataType parseType(spirv_cross::SPIRType& row_type);
uint32_t parseTypeSize(spirv_cross::SPIRType& row_type);
ProgramInfo* getProgramInfo(std::vector<uint32_t>& bin);

#endif