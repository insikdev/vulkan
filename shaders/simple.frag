#version 450

layout(set = 0, binding = 0) uniform ModelUniformData {
    mat4 world;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} model;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(set = 1, binding = 0) uniform CommonUniform
{
	mat4 view;
	mat4 proj;
    vec3 eyePos;
    vec3 lightPos;
    vec3 lightDir;
    vec3 lightColor;
} commonData;

layout(location = 0) in vec3 worldPos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 base = texture(texSampler, texCoord).xyz;
    //base = vec3(1.0, 0.5, 0.5);

    // Ambient
    vec3 ambient = model.ambient;

    // Diffuse
    vec3 N = normalize(normal);
    vec3 L = normalize(commonData.lightPos - worldPos);
    float diff = max(dot(N, L), 0.0);

    vec3 diffuse = diff * model.diffuse;
   
    // Specular
    vec3 eyePos = inverse(commonData.view)[3].xyz;
    vec3 viewDir = normalize(eyePos - worldPos);
    vec3 reflectDir = reflect(-L, N);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), model.shininess);
    
    vec3 specular = spec * model.specular;

    vec3 result = (ambient + diffuse + specular) * base;

    outColor = vec4(result, 1.0);
}