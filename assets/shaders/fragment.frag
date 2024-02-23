#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec2 fragUV;

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

layout(push_constant) uniform Push {
    mat4 modelMatrix;
} push;

layout(set = 1, binding = 0) uniform sampler2D shadowMap;

// material
layout(set = 2, binding = 0) uniform Material {
    vec4 base_color;
} material;
layout(set = 2, binding = 1) uniform sampler2D base_color_tex;

vec3 calcuPointLight(PointLight light) {
    vec3 dorectionToLight = light.position - fragPos;
    float lightDistance = length(dorectionToLight);
    dorectionToLight = normalize(dorectionToLight);
    vec3 lightColor = light.color.xyz * light.color.w;
    vec3 dorectionToCamera = normalize(ubo.inverseView[3].xyz - fragPos);
    
    float attenuation = 1 + 0.35 * lightDistance + 0.44 * lightDistance*lightDistance;
    
    vec3 diffuse = lightColor * max(dot(fragNormal, dorectionToLight), 0);
    
    vec3 halfAngle = normalize(dorectionToLight + dorectionToCamera);
    vec3 specular = lightColor * pow(dot(fragNormal, halfAngle), 114);
    
    return (diffuse + specular) / attenuation;
}

float calcuShadow() {
    vec4 posInLightSpace = ubo.lightSpace * vec4(fragPos, 1);
    vec3 projCoords = posInLightSpace.xyz / posInLightSpace.w;
    
    vec2 uv = projCoords.xy * 0.5 + 0.5;
    float currentDepth = projCoords.z;
    
    if (uv.x>1 || uv.y>1 || uv.x<0 || uv.y<0 || currentDepth >= 1)
        return 1; // 不在 shadow map 范围
    
    // 取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
    float closestDepth = 0;
    float dp = 3, b = 5e-4;
    for (float i = -dp; i <= dp; i += 1)
        for (float j = -dp; j <= dp; j += 1)
            closestDepth += texture(shadowMap, uv + vec2(i*b, j*b)).r;
    closestDepth /= (2*dp + 1) * (2*dp + 1);
    
    float shadow = smoothstep(closestDepth + 12e-3, closestDepth + 5e-3, currentDepth);
    
    return shadow;
}

void main(){
    vec3 lightColor = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    for (int i = 0; i < ubo.numLights; i++) {
        lightColor += calcuPointLight(ubo.pointLights[i]);
    }
    
    float shadow = calcuShadow() * 0.8 + 0.2;
    
    vec3 base_color = mix(texture(base_color_tex, fragUV).rgb, material.base_color.rgb, material.base_color.a);
    outColor = vec4(base_color * shadow * lightColor * fragColor, 1);
}
