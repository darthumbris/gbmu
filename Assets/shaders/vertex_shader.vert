#version 140

in vec2 LVertexPos2D;
out vec2 uv;

void main() {
    gl_Position = vec4(LVertexPos2D.x * 2 - 1.0, LVertexPos2D.y* 2 - 1.0, 0, 1);
    uv = vec2(LVertexPos2D.x, 1.0f - LVertexPos2D.y);
}
