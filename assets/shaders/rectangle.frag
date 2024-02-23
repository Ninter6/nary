#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 1) uniform sampler2D texSampler;

// float offsetx = 1e-3, offsety = 1e-3;
vec2 offset = 1 / textureSize(texSampler, 0);

vec2 offsets[9] = vec2[](
    vec2(-offset.x,  offset.y),   // 左上
    vec2( 0.0f,    offset.y),    // 正上
    vec2( offset.x,  offset.y),   // 右上
    vec2(-offset.x,  0.0f),      // 左
    vec2( 0.0f,    0.0f),       // 中
    vec2( offset.x,  0.0f),      // 右
    vec2(-offset.x, -offset.y),   // 左下
    vec2( 0.0f,   -offset.y),    // 正下
    vec2( offset.x, -offset.y)    // 右下
);

//float kernel[9] = float[](
//    1, 1, 1,
//    1,-8, 1,
//    1, 1, 1
//);
float kernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16
);

void main(){
    vec3 result = vec3(0);
    for(int i = 0; i < 9; i++)
        result += texture(texSampler, fragUV + offsets[i]).rgb * kernel[i];
    outColor = vec4(result, 1.0);
//    outColor = vec4(texture(texSampler, fragUV).rgb, 1);
}
