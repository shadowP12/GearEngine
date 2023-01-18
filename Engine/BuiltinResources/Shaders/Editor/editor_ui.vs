#define LOCATION_POSITION 0
#define LOCATION_COLOR 4
#define LOCATION_UV0 5
layout(location = LOCATION_POSITION) in vec4 mesh_position;
layout(location = LOCATION_COLOR) in vec4 mesh_color;
layout(location = LOCATION_UV0) in vec2 mesh_uv0;
layout(location = 0) out mediump vec4 vertex_color;
layout(location = 1) out mediump vec2 vertex_uv0;
layout(push_constant) uniform ConstantBlock {
    vec4 size;
} constant;

void main() {
    vertex_color = mesh_color;
    vertex_uv0 = mesh_uv0;
    gl_Position = vec4(mesh_position.x / constant.size.z, mesh_position.y / constant.size.w, 0.0f, 1.0f);
    gl_Position.x = gl_Position.x * 2.0 - 1.0;
    gl_Position.y = gl_Position.y * 2.0 - 1.0;
}