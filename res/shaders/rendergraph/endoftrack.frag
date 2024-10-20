#version 440

layout(location = 0) in float vGradient;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    vec4 color;
}
ubuf;

void main() {
    float minAlpha = 0.5 * ubuf.color.w;
    float maxAlpha = 0.83 * ubuf.color.w;
    float alpha = mix(minAlpha, maxAlpha, max(0.0, vGradient));
    // premultiple alpha
    fragColor = vec4(ubuf.color.xyz * alpha, alpha);
}
