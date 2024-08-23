#version 440

layout(location = 0) in vec4 position;
layout(location = 1) in float gradient;
layout(location = 0) out float vgradient;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vgradient = gradient;
    gl_Position = position;
}
