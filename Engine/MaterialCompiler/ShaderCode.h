#pragma once
namespace gear {
const char TEXTURE_FRAG_DATA[] = "#version 450#extension GL_ARB_separate_shader_objects : enablelayout(binding = 1) uniform sampler2D texSampler;layout(location = 0) in vec2 fragTexCoord;layout(location = 0) out vec4 outColor;void main(){    outColor = texture(texSampler, fragTexCoord);}";
const char TEXTURE_VERT_DATA[] = "#version 450#extension GL_ARB_separate_shader_objects : enablelayout(binding = 0) uniform UniformBufferObject {    mat4 model;    mat4 view;    mat4 proj;} ubo;layout(location = 0) in vec3 inPosition;layout(location = 1) in vec2 inTexCoord;layout(location = 0) out vec2 fragTexCoord;void main() {    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);    fragTexCoord = inTexCoord;}";
const char TRIANGLE_FRAG_DATA[] = "#version 450#extension GL_ARB_separate_shader_objects : enablelayout(location = 0) out vec4 outColor;void main(){    outColor = vec4(1.0, 0.0, 0.0, 0.0);}";
const char TRIANGLE_VERT_DATA[] = "#version 450#extension GL_ARB_separate_shader_objects : enablelayout(location = 0) in vec3 inPosition;layout(location = 1) in vec3 inNormal;layout(location = 2) in vec2 inTexCoord;void main() {    gl_Position = vec4(inPosition, 1.0);}";
}
