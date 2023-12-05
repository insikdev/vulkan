#version 450

layout(set = 0, binding = 0) uniform ModelUniformData {
    mat4 world;
} model;

layout(set = 1, binding = 0) uniform CommonUniform
{
	mat4 view;
	mat4 proj;
} common;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = common.proj * common.view * model.world * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}