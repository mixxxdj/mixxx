#version 310 es
precision mediump float;

layout(location = 0) in highp vec4 vColor;
layout(location = 0) out highp vec4 fragColor;

void main() {
    fragColor = vec4(vColor.xyz * vColor.w, vColor.w); // premultiple alpha
}
