#version 450

layout(set = 0, binding = 0) uniform ModelUniformData {
    mat4 world;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} model;

layout(set = 1, binding = 0) uniform CommonUniform
{
	mat4 view;
	mat4 proj;
    vec3 eyePos;
    vec3 lightPos;
    vec3 lightDir;
    vec3 lightColor;
} commonData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 worldPos;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 texCoord;

void main() {
    worldPos = (model.world * vec4(inPosition, 1.0)).xyz;
    normal = (transpose(inverse(model.world)) * vec4(inNormal, 0.0)).xyz;
    texCoord = inTexCoord;

    gl_Position = commonData.proj * commonData.view * model.world * vec4(inPosition, 1.0);
}