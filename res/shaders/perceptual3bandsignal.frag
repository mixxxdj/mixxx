uniform highp vec2 framebufferSize;
uniform highp vec4 axesColor;
uniform highp vec4 lowColor;
uniform highp vec4 midColor;
uniform highp vec4 highColor;
uniform highp vec4 overlapColor;

uniform int waveformLength;
uniform int textureSize;
uniform int textureStride;

uniform highp float allGain;
uniform highp float lowGain;
uniform highp float midGain;
uniform highp float highGain;
uniform highp float perceptualValueScale;
uniform highp float firstVisualIndex;
uniform highp float lastVisualIndex;

uniform sampler2D waveformDataTexture;

highp vec4 getWaveformData(highp float index) {
    highp vec2 uv_data;
    uv_data.y = floor(index / float(textureStride));
    uv_data.x = floor(index - uv_data.y * float(textureStride));
    return texture2D(waveformDataTexture, uv_data / float(textureStride));
}

highp vec4 getInterpolatedWaveformData(highp float visualIndex, highp float channelOffset) {
    highp float currentIndex = floor(visualIndex) * 2.0 + channelOffset;
    highp float lastIndexForChannel = float(waveformLength - 2) + channelOffset;
    highp float nextIndex = min(currentIndex + 2.0, lastIndexForChannel);
    return mix(getWaveformData(currentIndex),
            getWaveformData(nextIndex),
            fract(visualIndex));
}

void main(void) {
    highp vec2 uv = gl_TexCoord[0].st;
    highp vec4 pixel = gl_FragCoord;

    highp float currentVisualIndex =
            firstVisualIndex + uv.x * (lastVisualIndex - firstVisualIndex);
    highp float channelOffset = uv.y < 0.5 ? 1.0 : 0.0;
    highp float currentIndex = floor(currentVisualIndex) * 2.0 + channelOffset;

    highp vec4 outputColor = vec4(0.0, 0.0, 0.0, 0.0);
    bool showing = false;
    highp vec4 showingColor = vec4(0.0, 0.0, 0.0, 0.0);

    if (currentIndex >= 0.0 && currentIndex <= float(waveformLength - 1)) {
        highp vec4 currentData = clamp(getInterpolatedWaveformData(
                                               currentVisualIndex, channelOffset),
                                         0.0,
                                         1.0) *
                allGain * perceptualValueScale;
        currentData.x *= lowGain;
        currentData.y *= midGain;
        currentData.z *= highGain;

        highp float distanceFromCenter = abs((uv.y - 0.5) * 2.0);
        if (distanceFromCenter <= currentData.z) {
            showingColor = highColor;
            showing = true;
        } else if (distanceFromCenter <= min(currentData.x, currentData.y)) {
            showingColor = overlapColor;
            showing = true;
        } else if (distanceFromCenter <= currentData.x) {
            showingColor = lowColor;
            showing = true;
        } else if (distanceFromCenter <= currentData.y) {
            showingColor = midColor;
            showing = true;
        }
    }

    if (abs(framebufferSize.y / 2.0 - pixel.y) <= 4.0) {
        outputColor.xyz = mix(outputColor.xyz, axesColor.xyz, axesColor.w);
        outputColor.w = 1.0;
    }

    if (showing) {
        outputColor.xyz = showingColor.xyz;
        outputColor.w = 1.0;
    }

    gl_FragColor = outputColor;
}
