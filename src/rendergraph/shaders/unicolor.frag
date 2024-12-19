#version 440

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    vec4 color;
}
ubuf;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(ubuf.color.xyz * ubuf.color.w, ubuf.color.w); // premultiply alpha
}
