#version 440

layout(std140, binding = 0) uniform buf {
    highp mat4 matrix;
}
ubuf;

layout(location = 0) in highp vec4 position;
layout(location = 1) in highp vec2 texcoord;
layout(location = 0) out highp vec2 vTexcoord;

void main() {
    vTexcoord = texcoord;
    gl_Position = ubuf.matrix * position;
}
