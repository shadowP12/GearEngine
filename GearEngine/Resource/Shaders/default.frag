#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout (set = 0, binding = 1) uniform sampler2D diffuseMap;
layout(set = 0, binding = 2) uniform UboLight
{
	vec3 lightDir;
	vec3 lightColor;
} light;
void main()
{
	vec4 color = texture(diffuseMap, fragTexCoord);
	vec3 ambient = vec3(0.1, 0.1, 0.1);
	vec3 normal = normalize(fragNormal);
	vec3 lightDir = normalize(light.lightDir);
	float ndl = max(0.0,dot(normal,lightDir));
	vec3 diffuse = light.lightColor.xyz * ndl;
    outColor = vec4(diffuse * color.rgb + ambient, 1.0);
}