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

layout(set = 1, binding = 0) uniform sampler2D shadowMap;

// material
layout(set = 2, binding = 0) uniform Material {
    vec4 base_color;
    float metallic;
    float roughness;
} material;
layout(set = 2, binding = 1) uniform sampler2D base_color_tex;

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)  {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 calcuPointLight_BP(PointLight light) {
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

vec3 calcuLightPBR(vec3 albedo, float metallic, float roughness, vec3 N, vec3 V, vec3 L, vec3 H, vec3 radiance) {
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);

    vec3 nominator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular     = nominator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 calcuPointLight(PointLight light, vec3 albedo, float metallic, float roughness, vec3 N, vec3 V) {
    vec3 lightColor = light.color.rgb;

    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(V + L);

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance     = lightColor * attenuation;

    return calcuLightPBR(albedo, metallic, roughness, N, V, L, H, radiance);
}

vec3 calcuDirectionalLight(vec3 albedo, float metallic, float roughness, vec3 N, vec3 V) {
    vec3 L = normalize(ubo.directionalLight.direction);
    vec3 H = normalize(V + L);

    vec3 radiance = ubo.directionalLight.color.rgb;

    return calcuLightPBR(albedo, metallic, roughness, N, V, L, H, radiance);
}

float calcuShadow() {
    vec4 posInLightSpace = ubo.directionalLight.projView * vec4(fragPos, 1);
    vec3 projCoords = posInLightSpace.xyz / posInLightSpace.w;
    
    vec2 uv = projCoords.xy * 0.5 + 0.5;
    float currentDepth = projCoords.z;
    
//    if (uv.x>1 || uv.y>1 || uv.x<0 || uv.y<0 || currentDepth >= 1)
//        return 1; // 不在 shadow map 范围
    
    // 取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
    float closestDepth = 0;
    float dp = 3, b = 5e-4;
    for (float i = -dp; i <= dp; i += 1)
        for (float j = -dp; j <= dp; j += 1)
            closestDepth += texture(shadowMap, uv + vec2(i*b, j*b)).r;
    closestDepth /= (2*dp + 1) * (2*dp + 1);
    
    float shadow = smoothstep(closestDepth + 12e-3, closestDepth + 5e-4, currentDepth);
    
    return shadow;
}

void main(){
    vec3 albedo = mix(texture(base_color_tex, fragUV).rgb, material.base_color.rgb, material.base_color.a);
    float metallic = material.metallic;
    float roughness = material.roughness;

    vec3 N = normalize(fragNormal);
    vec3 V = normalize(ubo.inverseView[3].xyz - fragPos);

    vec3 lightColor = 0.03 * ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    for (int i = 0; i < ubo.numLights; i++) {
        lightColor += calcuPointLight(ubo.pointLights[i], albedo, metallic, roughness, N, V);
    }

    float shadow = calcuShadow();
    lightColor += shadow * calcuDirectionalLight(albedo, metallic, roughness, N, V);

    outColor = vec4(lightColor * fragColor, 1);
}
