#version 120

uniform vec2 framebufferSize;
uniform vec4 axesColor;
uniform vec4 lowColor;
uniform vec4 midColor;
uniform vec4 highColor;
uniform vec4 lowFilteredColor;
uniform vec4 midFilteredColor;
uniform vec4 highFilteredColor;

uniform int waveformLength;
uniform int textureSize;
uniform int textureStride;

uniform float allGain;
uniform float lowGain;
uniform float midGain;
uniform float highGain;
uniform float firstVisualIndex;
uniform float lastVisualIndex;

uniform sampler2D waveformDataTexture;

vec4 getWaveformData(float index) {
    vec2 uv_data;
    uv_data.y = floor(index / float(textureStride));
    uv_data.x = floor(index - uv_data.y * float(textureStride));
    // Divide again to convert to normalized UV coordinates.
    return texture2D(waveformDataTexture, uv_data / float(textureStride));
}

bool nearBorder(float target, float test, float epsilon) {
    float dist = target - test;
    return (abs(dist) <= epsilon && dist > 0);
}

void main(void) {
    vec2 uv = gl_TexCoord[0].st;
    vec4 pixel = gl_FragCoord;

    float new_currentIndex =
            floor(firstVisualIndex +
                    uv.x * (lastVisualIndex - firstVisualIndex)) *
            2;

    // Texture coordinates put (0,0) at the bottom left, so show the right
    // channel if we are in the bottom half.
    if (uv.y < 0.5) {
        new_currentIndex += 1;
    }

    vec4 outputColor = vec4(0.0, 0.0, 0.0, 0.0);
    bool showing = false;
    bool showingUnscaled = false;
    vec4 showingColor = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 mixColor = vec4(0.0, 0.0, 0.0, 0.0);
    bool mixin = true;
    float alpha = 0.75;

    // We don't exit early if the waveform data is not valid because we may want
    // to show other things (e.g. the axes lines) even when we are on a pixel
    // that does not have valid waveform data.
    if (new_currentIndex >= 0 && new_currentIndex <= waveformLength - 1) {
        // Since the magnitude of the (low, mid, high) vector is used as the
        // waveform height, re-scale the maximum height to 1.
        const float scaleFactor = 1.0 / sqrt(3.0);

        vec4 new_currentDataUnscaled = getWaveformData(new_currentIndex) * allGain;
        new_currentDataUnscaled.x *= scaleFactor;
        new_currentDataUnscaled.y *= scaleFactor;
        new_currentDataUnscaled.z *= scaleFactor;

        vec4 new_currentData = new_currentDataUnscaled;
        new_currentData.x *= lowGain;
        new_currentData.y *= midGain;
        new_currentData.z *= highGain;

        vec4 new_currentDataTop = vec4(0.0, 0.0, 0.0, 0.0);
        new_currentDataTop.x = max(new_currentData.x, new_currentDataUnscaled.x);
        new_currentDataTop.y = max(new_currentData.y, new_currentDataUnscaled.y);
        new_currentDataTop.z = max(new_currentData.z, new_currentDataUnscaled.z);

        // Represents the [-1, 1] distance of this pixel. Subtracting this from
        // the signal data in new_currentData, we can tell if a signal band should
        // show in this pixel if the component is > 0.
        float ourDistance = abs((uv.y - 0.5) * 2.0);

        float signalDistance = sqrt(new_currentData.x * new_currentData.x +
                                       new_currentData.y * new_currentData.y +
                                       new_currentData.z * new_currentData.z) *
                scaleFactor;

        bool drawBorder = false;
        showing = true;
        if (drawBorder && nearBorder(new_currentDataUnscaled.x, ourDistance, 0.04)) {
            showingColor = lowColor;
            mixin = false;
            alpha = 0.90;
        } else if (ourDistance <= new_currentData.x) {
            showingColor = lowColor;
        } else if (ourDistance <= new_currentDataUnscaled.x) {
            showingColor = lowFilteredColor;
            alpha = 0.6;
        } else if (drawBorder &&
                nearBorder(
                        new_currentDataUnscaled.x + new_currentDataUnscaled.y,
                        ourDistance,
                        0.04)) {
            showingColor = midColor;
            mixin = false;
            alpha = 0.90;
        } else if (ourDistance <= new_currentDataTop.x + new_currentData.y) {
            showingColor = midColor;
        } else if (ourDistance <= new_currentDataTop.x + new_currentDataTop.y) {
            showingColor = midFilteredColor;
            alpha = 0.6;
        } else if (drawBorder &&
                nearBorder(new_currentDataTop.x + new_currentDataTop.y +
                                new_currentDataUnscaled.z,
                        ourDistance,
                        0.04)) {
            showingColor = highColor;
            mixin = false;
            alpha = 0.90;
        } else if (ourDistance <= new_currentDataTop.x + new_currentDataTop.y +
                        new_currentData.z) {
            showingColor = highColor;
        } else if (ourDistance <= new_currentDataTop.x + new_currentDataTop.y +
                        new_currentDataUnscaled.z) {
            showingColor = highFilteredColor;
            alpha = 0.6;
        } else {
            showing = false;
        }

        // Linearly combine the low, mid, and high colors according to the low,
        // mid, and high components of the unfiltered signal.
        mixColor = lowColor * new_currentDataUnscaled.x +
                midColor * new_currentDataUnscaled.y +
                highColor * new_currentDataUnscaled.z;

        // Re-scale the color by the maximum component.
        float showingMax = max(mixColor.x, max(mixColor.y, mixColor.z));
        mixColor = mixColor / showingMax;
        mixColor.w = 1.0;
    }
    // Draw the axes color as the lowest item on the screen.
    // TODO(owilliams): The "4" in this line makes sure the axis gets
    // rendered even when the waveform is fairly short.  Really this
    // value should be based on the size of the widget.
    if (abs(framebufferSize.y / 2 - pixel.y) <= 4) {
        outputColor.xyz = mix(outputColor.xyz, axesColor.xyz, axesColor.w);
        outputColor.w = 1.0;
    }

    if (showing) {
        outputColor.xyz = mix(outputColor.xyz, showingColor.xyz, alpha);
        // we mix in the sum color to smoothen the look and give it the
        // general color tone.
        if (mixin == true) {
            outputColor.xyz = mix(outputColor.xyz, mixColor.xyz, 0.53f);
        }
        outputColor.w = 1.0;
    }

    gl_FragColor = outputColor;
}
