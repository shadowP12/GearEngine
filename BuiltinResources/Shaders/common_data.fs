layout(std140, set = 0, binding = 1) uniform FrameUniforms {
    mat4 view_matrix;
    mat4 proj_matrix;
} frame_uniforms;

layout(std140, set = 0, binding = 2) uniform ObjectUniforms {
    mat4 model_matrix;
    mat4 normal_matrix;
} object_uniforms;