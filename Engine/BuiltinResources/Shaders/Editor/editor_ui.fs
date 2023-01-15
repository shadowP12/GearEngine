layout(location = 0) in mediump vec4 vertex_color;
layout(location = 1) in mediump vec2 vertex_uv0;
layout(location = 0) out vec4 fragColor;
layout(binding = 1000) uniform texture2D texture;
layout(binding = 3000) uniform sampler sampler;

void main() {
    vec4 color = vertex_color * texture(sampler2D(texture, sampler), vertex_uv0);
    color.rgb *= color.a;
    fragColor = color;
}