#version 440

layout(location = 0) in vec4 position;
layout(location = 1) in float gradient;
layout(location = 0) out float vGradient;

void main() {
    vGradient = gradient;
    gl_Position = position;
}
