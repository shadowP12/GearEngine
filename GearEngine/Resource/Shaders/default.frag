#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout (set = 0, binding = 1) uniform sampler2D diffuseMap;

void main()
{
    outColor = texture(diffuseMap, fragTexCoord);
}