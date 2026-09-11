#version 120

varying highp vec2 vTexcoord;

uniform vec2 framebufferSize;
uniform vec4 axesColor;
uniform vec4 lowColor;
uniform vec4 midColor;
uniform vec4 highColor;

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
    vec2 uv = vTexcoord;
    vec4 pixel = gl_FragCoord;

    float new_currentIndex = floor(firstVisualIndex + uv.x *
                                   (lastVisualIndex - firstVisualIndex)) * 2;

    // Texture coordinates put (0,0) at the bottom left, so show the right
    // channel if we are in the bottom half.
    if (uv.y < 0.5) {
        new_currentIndex += 1;
    }

    vec4 outputColor = vec4(0.0, 0.0, 0.0, 0.0);
    bool lowShowing = false;
    bool midShowing = false;
    bool highShowing = false;
    bool lowShowingUnscaled = false;
    bool midShowingUnscaled = false;
    bool highShowingUnscaled = false;
    // We don't exit early if the waveform data is not valid because we may want
    // to show other things (e.g. the axes lines) even when we are on a pixel
    // that does not have valid waveform data.
    if (new_currentIndex >= 0 && new_currentIndex <= waveformLength - 1) {
      vec4 new_currentDataUnscaled = getWaveformData(new_currentIndex) * allGain;
      vec4 new_currentData = new_currentDataUnscaled;

      new_currentData.x *= lowGain;
      new_currentData.y *= midGain;
      new_currentData.z *= highGain;

      // Represents the [-1, 1] distance of this pixel. Subtracting this from
      // the signal data in new_currentData, we can tell if a signal band should
      // show in this pixel if the component is > 0.
      float ourDistance = abs((uv.y - 0.5) * 2.0);

      vec4 signalDistance = new_currentData - ourDistance;
      lowShowing = signalDistance.x >= 0.0;
      midShowing = signalDistance.y >= 0.0;
      highShowing = signalDistance.z >= 0.0;

      // Now do it all over again for the unscaled version of the waveform,
      // which we will draw at very low opacity.
      vec4 signalDistanceUnscaled = new_currentDataUnscaled - ourDistance;
      lowShowingUnscaled = signalDistanceUnscaled.x >= 0.0;
      midShowingUnscaled = signalDistanceUnscaled.y >= 0.0;
      highShowingUnscaled = signalDistanceUnscaled.z >= 0.0;
    }

    // Draw the axes color as the lowest item on the screen.
    // TODO(owilliams): The "4" in this line makes sure the axis gets
    // rendered even when the waveform is fairly short.  Really this
    // value should be based on the size of the widget.
    if (abs(framebufferSize.y / 2 - pixel.y) <= 4) {
      outputColor.xyz = mix(outputColor.xyz, axesColor.xyz, axesColor.w);
      outputColor.w = 1.0;
    }

    if (lowShowingUnscaled) {
      float lowAlpha = 0.2;
      outputColor.xyz = mix(outputColor.xyz, lowColor.xyz, lowAlpha);
      outputColor.w = 1.0;
    }
    if (midShowingUnscaled) {
      float midAlpha = 0.2;
      outputColor.xyz = mix(outputColor.xyz, midColor.xyz, midAlpha);
      outputColor.w = 1.0;
    }
    if (highShowingUnscaled) {
      float highAlpha = 0.2;
      outputColor.xyz = mix(outputColor.xyz, highColor.xyz, highAlpha);
      outputColor.w = 1.0;
    }

    if (lowShowing) {
      float lowAlpha = 0.8;
      outputColor.xyz = mix(outputColor.xyz, lowColor.xyz, lowAlpha);
      outputColor.w = 1.0;
    }

    if (midShowing) {
      float midAlpha = 0.85;
      outputColor.xyz = mix(outputColor.xyz, midColor.xyz, midAlpha);
      outputColor.w = 1.0;
    }

    if (highShowing) {
      float highAlpha = 0.9;
      outputColor.xyz = mix(outputColor.xyz, highColor.xyz, highAlpha);
      outputColor.w = 1.0;
    }

    gl_FragColor = outputColor;
}
