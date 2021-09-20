#pragma once
namespace gear {
const char COMMON_DATA_FS_DATA[] = "layout(std140, set = 0, binding = 1) uniform FrameUniforms {\n    mat4 view_matrix;\n    mat4 proj_matrix;\n    mat4 sun_matrixs[SHADOW_CASCADE_COUNT];\n    vec4 sun_direction;\n} frame_uniforms;\n\nlayout(std140, set = 0, binding = 2) uniform ObjectUniforms {\n    mat4 model_matrix;\n    mat4 normal_matrix;\n} object_uniforms;\n\n#if defined(HAS_SHADOWING)\nlayout(set = 1, binding = 0) uniform sampler2DArray cascade_shadow_map;\n#endif";
const char DEPTH_MAIN_FS_DATA[] = "void main() {\n}\n";
const char DEPTH_MAIN_VS_DATA[] = "void main() {\n    MaterialVertexParams material_params;\n\n    InitMaterialVertexParams(material_params);\n\n    ProcessMaterialVertexParams(material_params);\n\n    gl_Position = frame_uniforms.proj_matrix * frame_uniforms.view_matrix * material_params.world_position;\n}\n";
const char INPUTS_FS_DATA[] = "layout(location = 0) in highp vec3 vertex_world_position;\n\n#if defined(HAS_ATTRIBUTE_UV0) && !defined(HAS_ATTRIBUTE_UV1)\nlayout(location = 5) in highp vec2 vertex_uv01;\n#elif defined(HAS_ATTRIBUTE_UV1)\nlayout(location = 5) in highp vec4 vertex_uv01;\n#endif\n\n#if defined(HAS_ATTRIBUTE_NORMAL)\nlayout(location = 6) in highp vec3 vertex_normal;\n#endif\n\n#if defined(HAS_ATTRIBUTE_COLOR)\nlayout(location = 7) in mediump vec4 vertex_color;\n#endif";
const char INPUTS_VS_DATA[] = "layout(location = LOCATION_POSITION) in vec4 mesh_position;\n\n#if defined(HAS_ATTRIBUTE_NORMAL)\nlayout(location = LOCATION_NORMAL) in vec3 mesh_normal;\nlayout(location = LOCATION_TANGENT) in vec3 mesh_tangent;\nlayout(location = LOCATION_BITANGENT) in vec3 mesh_bitangent;\n#endif\n\n#if defined(HAS_ATTRIBUTE_COLOR)\nlayout(location = LOCATION_COLOR) in vec4 mesh_color;\n#endif\n\n#if defined(HAS_ATTRIBUTE_UV0)\nlayout(location = LOCATION_UV0) in vec2 mesh_uv0;\n#endif\n\n#if defined(HAS_ATTRIBUTE_UV1)\nlayout(location = LOCATION_UV1) in vec2 mesh_uv1;\n#endif\n\n#if defined(HAS_ATTRIBUTE_BONE_INDICES)\nlayout(location = LOCATION_BONE_INDICES) in uvec4 mesh_bone_indices;\nlayout(location = LOCATION_BONE_WEIGHTS) in vec4 mesh_bone_weights;\n#endif\n\n\n#if defined(HAS_ATTRIBUTE_CUSTOM0)\nlayout(location = LOCATION_CUSTOM0) in vec4 mesh_custom0;\n#endif\n\n#if defined(HAS_ATTRIBUTE_CUSTOM1)\nlayout(location = LOCATION_CUSTOM1) in vec4 mesh_custom1;\n#endif\n\n#if defined(HAS_ATTRIBUTE_CUSTOM2)\nlayout(location = LOCATION_CUSTOM2) in vec4 mesh_custom2;\n#endif\n\n#if defined(HAS_ATTRIBUTE_CUSTOM3)\nlayout(location = LOCATION_CUSTOM3) in vec4 mesh_custom3;\n#endif\n\n#if defined(HAS_ATTRIBUTE_CUSTOM4)\nlayout(location = LOCATION_CUSTOM4) in vec4 mesh_custom4;\n#endif\n\n#if defined(HAS_ATTRIBUTE_CUSTOM5)\nlayout(location = LOCATION_CUSTOM5) in vec4 mesh_custom5;\n#endif\n\nlayout(location = 0) out highp vec3 vertex_world_position;\n\nlayout(location = 1) out highp vec4 vertex_position;\n\n#if defined(HAS_ATTRIBUTE_UV0) && !defined(HAS_ATTRIBUTE_UV1)\nlayout(location = 5) out highp vec2 vertex_uv01;\n#elif defined(HAS_ATTRIBUTE_UV1)\nlayout(location = 5) out highp vec4 vertex_uv01;\n#endif\n\n#if defined(HAS_ATTRIBUTE_NORMAL)\nlayout(location = 6) out highp vec3 vertex_normal;\n#endif\n\n#if defined(HAS_ATTRIBUTE_COLOR)\nlayout(location = 7) out mediump vec4 vertex_color;\n#endif\n";
const char LIGHT_DIRECTIONAL_FS_DATA[] = "void EvaluateDirectionalLight(const MaterialFragmentParams material_params, inout vec3 color) {\n    color = material_params.base_color.xyz;\n\n#if defined(HAS_ATTRIBUTE_NORMAL)\n    color = max(dot(vertex_normal, -frame_uniforms.sun_direction.xyz), 0.0) * vec3(0.6);\n#endif\n\n#if defined(HAS_SHADOWING)\n    vec4 light_space_position = (frame_uniforms.sun_matrixs[0] * vec4(vertex_world_position, 1.0));\n    vec3 proj_coords = light_space_position.xyz / light_space_position.w;\n    proj_coords.xy = proj_coords.xy * 0.5 + 0.5;\n    float closest_depth = texture(cascade_shadow_map, vec3(proj_coords.x, proj_coords.y, 0)).r; \n    float current_depth = clamp(proj_coords.z, 0.0, 1.0);\n\n    float bias = 0.0005;\n#if defined(HAS_ATTRIBUTE_NORMAL)\n    bias = max(0.003 * (1.0 - dot(normalize(vertex_normal), normalize(frame_uniforms.sun_direction.xyz))), 0.0005);\n#endif\n\n    float visibility = current_depth - bias > closest_depth ? 0.5 : 0.0;\n    color *= (1.0 - visibility);\n    color += 0.3;\n#endif\n}";
const char LIGHT_PUNCTUAL_FS_DATA[] = "void EvaluatePunctualLights(const MaterialFragmentParams material_params, inout vec3 color) {\n    color = material_params.base_color.xyz;\n}";
const char MAIN_FS_DATA[] = "layout(location = 0) out vec4 fragColor;\n\nvoid main() {\n    MaterialFragmentParams material_params;\n\n    InitMaterialFragmentParams(material_params);\n\n    ProcessMaterialFragmentParams(material_params);\n    \n    fragColor = EvaluateMaterial(material_params);\n}\n";
const char MAIN_VS_DATA[] = "void main() {\n    MaterialVertexParams material_params;\n\n    InitMaterialVertexParams(material_params);\n\n    ProcessMaterialVertexParams(material_params);\n\n    vertex_position = mesh_position;\n\n    vertex_world_position = material_params.world_position.xyz;\n\n#if defined(HAS_ATTRIBUTE_UV0)\n    vertex_uv01.xy = material_params.uv0;\n#endif\n#if defined(HAS_ATTRIBUTE_UV1)\n    vertex_uv01.zw = material_params.uv1;\n#endif\n\n#if defined(HAS_ATTRIBUTE_COLOR)\n    vertex_color = material_params.color;\n#endif\n\n#if defined(HAS_ATTRIBUTE_NORMAL)\n    vertex_normal = material_params.world_normal;\n#endif\n\n    gl_Position = frame_uniforms.proj_matrix * frame_uniforms.view_matrix * material_params.world_position;\n}\n";
const char MATERIAL_PARAMS_FS_DATA[] = "struct MaterialFragmentParams {\n    vec4  base_color;\n};\n\nvoid InitMaterialFragmentParams(out MaterialFragmentParams params) {\n    params.base_color = vec4(1.0);\n}";
const char MATERIAL_PARAMS_VS_DATA[] = "struct MaterialVertexParams {\n    vec4 world_position;\n\n#ifdef HAS_ATTRIBUTE_NORMAL\n    vec3 world_normal;\n#endif\n\n#ifdef HAS_ATTRIBUTE_COLOR\n    vec4 color;\n#endif\n\n#ifdef HAS_ATTRIBUTE_UV0\n    vec2 uv0;\n#endif\n\n#ifdef HAS_ATTRIBUTE_UV1\n    vec2 uv1;\n#endif\n};\n\nvoid InitMaterialVertexParams(out MaterialVertexParams params) {\n    params.world_position = object_uniforms.model_matrix * mesh_position;\n\n#ifdef HAS_ATTRIBUTE_COLOR\n    params.color = mesh_color;\n#endif\n\n#ifdef HAS_ATTRIBUTE_UV0\n    params.uv0 = mesh_uv0;\n#endif\n\n#ifdef HAS_ATTRIBUTE_UV1\n    params.uv1 = mesh_uv1;\n#endif\n\n#ifdef HAS_ATTRIBUTE_NORMAL\n    params.world_normal = normalize(mat3(object_uniforms.normal_matrix) * mesh_normal);\n#endif\n}\n";
const char SHADING_LIT_FS_DATA[] = "vec4 EvaluateLights(const MaterialFragmentParams material_params) {\n    vec3 color = vec3(0.0);\n\n#if defined(HAS_DIRECTIONAL_LIGHTING)\n    EvaluateDirectionalLight(material_params, color);\n#endif\n\n#if defined(HAS_DYNAMIC_LIGHTING)\n    EvaluatePunctualLights(material_params, color);\n#endif\n\n    return vec4(color, material_params.base_color.a);\n}\n\nvec4 EvaluateMaterial(const MaterialFragmentParams material_params) {\n    vec4 color = EvaluateLights(material_params);\n    return color;\n}";
const char SHADING_UNLIT_FS_DATA[] = "vec4 EvaluateMaterial(const MaterialFragmentParams material_params) {\n    vec4 color = material_params.base_color;\n    return color;\n}";
}
