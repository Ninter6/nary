#version 450

layout(location = 0) in vec2 fragOffset;
layout(location = 1) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

struct PointLight {
    vec3 position;
    float radius;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    mat4 lightSpace;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

void main() {
    float dis = length(fragOffset);
    if (dis > 1) discard;
    outColor = vec4(fragColor, 1 - 3 * dis*dis + 2 * dis*dis*dis);
}
