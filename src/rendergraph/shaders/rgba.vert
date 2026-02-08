#version 440

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
}
ubuf;

layout(location = 0) in highp vec4 position;
layout(location = 1) in highp vec4 color;
layout(location = 0) out highp vec4 vColor;

void main() {
    vColor = color;
    gl_Position = ubuf.matrix * position;
}
