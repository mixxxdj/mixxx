#version 440

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    float alpha;
}
ubuf;

layout(binding = 1) uniform sampler2D texture1;
layout(location = 0) in vec2 vTexcoord;
layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = texture(texture1, vTexcoord);
    if (ubuf.alpha > 0.0) {
        fragColor *= ubuf.alpha;
    }
}
