#version 440

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
}
ubuf;

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 color;
layout(location = 0) out vec3 vColor;

void main() {
    vColor = color;
    gl_Position = ubuf.matrix * position;
}
