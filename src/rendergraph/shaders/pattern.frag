#version 440

layout(binding = 1) uniform sampler2D texture1;
layout(location = 0) in vec2 vTexcoord;
layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = texture(texture1, fract(vTexcoord));
}
