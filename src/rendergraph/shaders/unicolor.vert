#version 440

layout(std140, binding = 0) uniform buf {
    highp mat4 matrix;
    highp vec4 color;
}
ubuf;

layout(location = 0) in vec4 position;

void main() {
    gl_Position = ubuf.matrix * position;
}
