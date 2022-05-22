layout(std140, set = 0, binding = 1) uniform FrameUniforms {
    mat4 view_matrix;
    mat4 main_view_matrix;
    mat4 proj_matrix;
    mat4 sun_matrixs[SHADOW_CASCADE_COUNT];
    vec4 sun_direction;
    vec4 sun_color_intensity;
    vec4 view_position;
    vec4 cascade_splits;
    float ev100;
    float exposure;
} frame_uniforms;

layout(std140, set = 0, binding = 2) uniform ObjectUniforms {
    mat4 local_matrix;
    mat4 model_matrix;
    mat4 normal_matrix;
} object_uniforms;

#if defined(HAS_SKINNING_OR_MORPHING)
#define MAX_NUM_JOINTS 64

layout(std140, set = 0, binding = 3) uniform BoneUniforms {
    mat4 joint_matrixs[MAX_NUM_JOINTS];
} bone_uniforms;

#endif

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

#define PI                 3.14159265359

#define HALF_PI            1.570796327

#define saturate(x)        clamp(x, 0.0, 1.0)

#define atan2(x, y)        atan(y, x)

struct Light {
    vec4 color_intensity;
    float attenuation;
    vec3 world_position;
    vec3 L;
};

// Global Params
vec3 shading_view;

vec3 shading_normal;