#version 450

layout(location = 0) out vec2 fragUV;

vec2 vertices[] = vec2[](
    vec2(-1, -1),
    vec2(-1, 1),
    vec2(1, 1),
    vec2(-1, -1),
    vec2(1, 1),
    vec2(1, -1)
);

void main() {
    fragUV = (vertices[gl_VertexIndex] + 1) / 2;
    gl_Position = vec4(vertices[gl_VertexIndex], 0, 1);
}
