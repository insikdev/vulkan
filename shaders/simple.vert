#version 450

layout(binding = 0) uniform ModelUniformData {
    mat4 world;
} model;

layout(push_constant) uniform constants
{
	mat4 view;
	mat4 proj;
} camera;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = camera.proj * camera.view * model.world * vec4(inPosition, 1.0);
    fragColor = inColor;
}