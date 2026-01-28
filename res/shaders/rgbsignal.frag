#version 120

uniform vec2 framebufferSize;
uniform vec4 axesColor;
uniform vec4 lowColor;
uniform vec4 midColor;
uniform vec4 highColor;
uniform bool splitStereoSignal;

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

void main(void) {
    vec2 uv = gl_TexCoord[0].st;
    float pixelY = gl_FragCoord.y;

    float indexRange = lastVisualIndex - firstVisualIndex;
    float currentIndex = floor(firstVisualIndex + uv.x * indexRange) * 2.0;

    vec4 outputColor = vec4(0.0, 0.0, 0.0, 0.0);

    bool showing = false;
    bool shadow = false;

    if (currentIndex >= 0.0 && currentIndex <= float(waveformLength - 1)) {
        vec4 dataUnscaled;

        if (splitStereoSignal) {
            // Texture coordinates put (0,0) at the bottom left, so show the right
            // channel if we are in the bottom half.
            float stereoIndex = (uv.y < 0.5) ? currentIndex + 1.0 : currentIndex;
            dataUnscaled = getWaveformData(stereoIndex);
        } else {
            vec4 left = getWaveformData(currentIndex);
            vec4 right = getWaveformData(currentIndex + 1.0);
            dataUnscaled = max(left, right);
        }

        dataUnscaled *= allGain;
        vec3 data = dataUnscaled.xyz * vec3(lowGain, midGain, highGain);

        // ourDistance represents the [0, 1] distance of this pixel from the
        // center line. If ourDistance is smaller than the signalDistance, show
        // the pixel.
        float ourDistance = abs(uv.y - 0.5) * 2.0;

        float sumUnscaled = dataUnscaled.x + dataUnscaled.y + dataUnscaled.z;
        float sumScaled = data.x + data.y + data.z;

        float signalDistance = dataUnscaled.w;
        if (sumUnscaled > 0.0) {
            signalDistance *= sumScaled / sumUnscaled;
        }

        showing = signalDistance >= ourDistance;
        shadow = !showing && (dataUnscaled.w >= ourDistance);

        // Linearly combine the low, mid, and high colors according to the low,
        // mid, and high components.
        if (showing) {
            outputColor =
                    lowColor * data.x +
                    midColor * data.y +
                    highColor * data.z;
        } else if (shadow) {
            outputColor =
                    lowColor * dataUnscaled.x +
                    midColor * dataUnscaled.y +
                    highColor * dataUnscaled.z;
        }

        float maxComponent = max(outputColor.x,
                max(outputColor.y, outputColor.z));

        if (maxComponent > 0.0) {
            outputColor.xyz /= maxComponent;
        }
    }

    if (showing) {
        outputColor.w = 1.0;
    } else if (abs(framebufferSize.y / 2 - pixelY) <= 4.0) {
        // Draw the axes color as the lowest item on the screen.
        // TODO(owilliams): The "4" in this line makes sure the axis gets
        // rendered even when the waveform is fairly short.  Really this
        // value should be based on the size of the widget.
        outputColor = axesColor;
    } else if (shadow) {
        outputColor.w = 0.4;
    }

    gl_FragColor = outputColor;
}
