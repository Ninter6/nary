#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

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
    gl_Position = ubo.directionalLight.projView * push.modelMatrix * vec4(position, 1);
}
