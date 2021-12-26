layout(std140, set = 0, binding = 1) uniform FrameUniforms {
    mat4 view_matrix;
    mat4 main_view_matrix;
    mat4 proj_matrix;
    mat4 sun_matrixs[SHADOW_CASCADE_COUNT];
    vec4 sun_direction;
    vec4 view_position;
    vec4 cascade_splits;
} frame_uniforms;

layout(std140, set = 0, binding = 2) uniform ObjectUniforms {
    mat4 model_matrix;
    mat4 normal_matrix;
} object_uniforms;

#if defined(HAS_SHADOWING)
layout(set = 0, binding = 1010) uniform texture2DArray cascade_shadow_map;
layout(set = 0, binding = 3010) uniform sampler cascade_shadow_sampler;
#endif

#if defined(HAS_IBL)
layout(set = 0, binding = 1011) uniform textureCube irradiance_map;
layout(set = 0, binding = 1012) uniform textureCube prefiltered_map;
layout(set = 0, binding = 1013) uniform texture2D brdf_lut;
layout(set = 0, binding = 3011) uniform sampler ibl_sampler;
#endif