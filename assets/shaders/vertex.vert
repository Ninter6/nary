#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragPos;
layout(location = 3) out vec2 fragUV;

struct PointLight {
    vec3 position;
    float radius;
    vec4 color;
};

struct DriectionalLight {
    mat4 projView;
    vec4 color;
    vec3 direction;
    float _padding;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientLightColor;
    DriectionalLight directionalLight;
    PointLight pointLights[10];
    int numLights;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
} push;

void main(){
    fragPos = (push.modelMatrix * vec4(position, 1)).xyz;
    gl_Position = ubo.projection * ubo.view * vec4(fragPos, 1);
    
    fragColor = color;
    fragNormal = normalize(transpose(inverse(mat3(push.modelMatrix))) * normal);
    fragUV = uv;
}
