#version 440

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
}
ubuf;

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 0) out vec4 vColor;

void main() {
    vColor = color;
    gl_Position = ubuf.matrix * position;
}
