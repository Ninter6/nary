#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler; // frame image

vec2 offset = 1 / textureSize(texSampler, 0);

vec2 offsets[9] = vec2[](
    vec2(-offset.x,  offset.y),     // 左上
    vec2( 0.0f,    offset.y),       // 正上
    vec2( offset.x,  offset.y),     // 右上
    vec2(-offset.x,  0.0f),         // 左
    vec2( 0.0f,    0.0f),           // 中
    vec2( offset.x,  0.0f),         // 右
    vec2(-offset.x, -offset.y),     // 左下
    vec2( 0.0f,   -offset.y),       // 正下
    vec2( offset.x, -offset.y)      // 右下
);

float brightness(vec3 color) {
    return 0.213 * color.r + 0.715 * color.g + 0.072 * color.b;
}

const float _MinThreshold = 0.3, _Threshold = 0.5; // threshold brightness
const int _SearchSteps = 10, _Guess = 8;

void main() {
    float luma[9]; // 0:NW 1:N 2:NE 3:W 4:M 5:E 6:SW 7:S 8:SE
    int norm[4] = int[4](1, 3, 5, 7); // N, W, E, S
    for (int i = 0; i < 9; ++i)
        luma[i] = brightness(texture(texSampler, fragUV + offsets[i]).rgb);
    
    float MaxLuma = 0, MinLuma = 114514;
    for (int i = 0; i < 4; ++i) {
        MaxLuma = max(luma[norm[i]], MaxLuma);
        MinLuma = min(luma[norm[i]], MinLuma);
    }
    
    float Contrast =  MaxLuma - MinLuma;
    if(Contrast >= max(_MinThreshold, MaxLuma * _Threshold)) {
        // Filter = 2 * (N + E + S + W) + NE + NW + SE + SW;
        // Filter = Filter / 12;
        // Filter = abs(Filter -  M);
        // Filter = saturate(Filter / Contrast);
        float Filter = 0;
        for (int i = 0; i < 4; ++i) Filter += luma[norm[i]];
        for (int i = 0; i < 9; ++i) Filter += luma[i];
        Filter = abs(Filter - 13 * luma[4]) / 12;
        Filter = clamp(0, Filter / Contrast, 1);
        
        float PixelBlend = pow(smoothstep(0, 1, Filter), 2);
        
        float Vertical = abs(luma[1] + luma[7] - 2 * luma[4]) * 2+ abs(luma[2] + luma[8] - 2 * luma[5]) + abs(luma[0] + luma[6] - 2 * luma[3]);
        float Horizontal = abs(luma[3] + luma[5] - 2 * luma[4]) * 2 + abs(luma[0] + luma[2] - 2 * luma[1]) + abs(luma[6] + luma[8] - 2 * luma[7]);
        bool IsHorizontal = Vertical > Horizontal;
        
        vec2 PixelStep = IsHorizontal ? vec2(0, offset.y) : vec2(offset.x, 0);
        
        float Positive = abs((IsHorizontal ? luma[1] : luma[5]) - luma[4]);
        float Negative = abs((IsHorizontal ? luma[7] : luma[3]) - luma[4]);
        float Gradient, OppositeLuminance;
        
        if(Positive > Negative) {
            Gradient = Positive;
            OppositeLuminance = IsHorizontal ? luma[1] : luma[5];
        } else {
            PixelStep = -PixelStep;
            Gradient = Negative;
            OppositeLuminance = IsHorizontal ? luma[7] : luma[3];
        }
        
        vec2 UVEdge = fragUV;
        UVEdge += PixelStep * 0.5f;
        vec2 EdgeStep = IsHorizontal ? vec2(offset.x, 0) : vec2(0, offset.y);
        
        float EdgeLuminance = (luma[4] + OppositeLuminance) * 0.5f;
        float GradientThreshold = EdgeLuminance * 0.25f;
        float PLuminanceDelta, NLuminanceDelta, PDistance, NDistance;
        int i;
        // 沿着锯齿方向搜索
        for(i = 1; i <= _SearchSteps; ++i) {
            PLuminanceDelta = brightness(texture(texSampler, UVEdge + i * EdgeStep).rgb) - EdgeLuminance;
            if(abs(PLuminanceDelta) > GradientThreshold) {
                PDistance = i * (IsHorizontal ? EdgeStep.x : EdgeStep.y);
                break;
            }
        }
        if(i == _SearchSteps + 1) {
            PDistance = (IsHorizontal ? EdgeStep.x : EdgeStep.y) * _Guess;
        }
        // 沿着另一侧锯齿方向搜索
        for(i = 1; i <= _SearchSteps; ++i) {
            NLuminanceDelta = brightness(texture(texSampler, UVEdge - i * EdgeStep).rgb) - EdgeLuminance;
            if(abs(NLuminanceDelta) > GradientThreshold) {
                NDistance = -i * (IsHorizontal ? EdgeStep.x : EdgeStep.y);
                break;
            }
        }
        if(i == _SearchSteps + 1) {
            NDistance = -(IsHorizontal ? EdgeStep.x : EdgeStep.y) * _Guess;
        }
        
        float EdgeBlend;
        if (PDistance < NDistance) {
            if(sign(PLuminanceDelta) == sign(luma[4] - EdgeLuminance)) {
                EdgeBlend = 0;
            } else {
                EdgeBlend = 0.5f - PDistance / (PDistance + NDistance);
            }
        } else {
            if(sign(NLuminanceDelta) == sign(luma[4] - EdgeLuminance)) {
                EdgeBlend = 0;
            } else {
                EdgeBlend = 0.5f - NDistance / (PDistance + NDistance);
            }
        }
        
        float FinalBlend = max(PixelBlend, EdgeBlend);
        
        outColor = vec4(texture(texSampler, fragUV + PixelStep * FinalBlend).rgb, 1);
    }
    
    outColor = vec4(texture(texSampler, fragUV).rgb, 1);
}
