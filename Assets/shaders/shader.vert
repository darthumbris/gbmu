#version 430

layout(location = 0) in vec2 texcoord;
layout(location = 0) out vec2 uv;

void main() {
    gl_Position = vec4(texcoord.x * 2 - 1.0, texcoord.y * 2 - 1.0, 0, 1);
    uv = vec2(texcoord.x, 1.0f - texcoord.y);
}
