#version 440

layout(location = 0) in highp vec3 vColor;
layout(location = 0) out highp vec4 fragColor;

void main() {
    fragColor = vec4(vColor, 1.0);
}
