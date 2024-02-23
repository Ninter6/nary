#version 450

const vec2 OFFSETS[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
);

layout(location = 0) out vec2 fragOffset;
layout(location = 1) out vec3 fragColor;

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

const float LIGHT_RADIUS = 0.1;

void main() {
    fragOffset = OFFSETS[gl_VertexIndex];
    vec4 lightInCameraSpace = ubo.view * vec4(ubo.pointLights[gl_InstanceIndex].position, 1.0);
    vec4 positionInCameraSpace = lightInCameraSpace + LIGHT_RADIUS * vec4(fragOffset, 0, 0);
    gl_Position = ubo.projection * positionInCameraSpace;
    
    fragColor = ubo.pointLights[gl_InstanceIndex].color.rgb * pow(ubo.pointLights[gl_InstanceIndex].color.g, 1 / 3);
}
