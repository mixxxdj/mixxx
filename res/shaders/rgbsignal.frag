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
    vec4 pixel = gl_FragCoord;

    float new_currentIndex = floor(firstVisualIndex + uv.x *
                                   (lastVisualIndex - firstVisualIndex)) * 2;

    vec4 outputColor = vec4(0.0, 0.0, 0.0, 0.0);
    bool showing = false;
    bool showingUnscaled = false;
    vec4 showingColor = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 showingUnscaledColor = vec4(0.0, 0.0, 0.0, 0.0);

    // We don't exit early if the waveform data is not valid because we may want
    // to show other things (e.g. the axes lines) even when we are on a pixel
    // that does not have valid waveform data.
    if (new_currentIndex >= 0 && new_currentIndex <= waveformLength - 1) {
        vec4 new_currentDataUnscaled;
        if (splitStereoSignal) {
            // Texture coordinates put (0,0) at the bottom left, so show the right
            // channel if we are in the bottom half.
            new_currentDataUnscaled = getWaveformData(uv.y < 0.5 ? new_currentIndex + 1 : new_currentIndex) * allGain;
        } else {
            vec4 leftDataUnscaled = getWaveformData(new_currentIndex);
            vec4 rightDataUnscaled = getWaveformData(new_currentIndex + 1);
            new_currentDataUnscaled = max(leftDataUnscaled, rightDataUnscaled) * allGain;
        }

      vec4 new_currentData = new_currentDataUnscaled;
      new_currentData.x *= lowGain;
      new_currentData.y *= midGain;
      new_currentData.z *= highGain;

      //(vrince) debug see pre-computed signal
      //gl_FragColor = new_currentData;
      //return;

      // Represents the [-1, 1] distance of this pixel. Subtracting this from
      // the signal data in new_currentData, we can tell if a signal band should
      // show in this pixel if the component is > 0.
      float ourDistance = abs((uv.y - 0.5) * 2.0);

      // Since the magnitude of the (low, mid, high) vector is used as the
      // waveform height, re-scale the maximum height to 1.
      const float scaleFactor = 1.0 / sqrt(3.0);

      float signalDistance = sqrt(new_currentData.x * new_currentData.x +
                                  new_currentData.y * new_currentData.y +
                                  new_currentData.z * new_currentData.z) *
                             scaleFactor;
      showing = (signalDistance - ourDistance) >= 0.0;

      // Linearly combine the low, mid, and high colors according to the low,
      // mid, and high components.
      showingColor = lowColor * new_currentData.x +
                     midColor * new_currentData.y +
                     highColor * new_currentData.z;

      // Re-scale the color by the maximum component.
      float showingMax = max(showingColor.x, max(showingColor.y, showingColor.z));
      showingColor = showingColor / showingMax;
      showingColor.w = 1.0;

      // Now do it all over again for the unscaled version of the waveform,
      // which we will draw at very low opacity.
      float signalDistanceUnscaled = sqrt(new_currentDataUnscaled.x * new_currentDataUnscaled.x +
                                          new_currentDataUnscaled.y * new_currentDataUnscaled.y +
                                          new_currentDataUnscaled.z * new_currentDataUnscaled.z) * scaleFactor;
      showingUnscaled = (signalDistanceUnscaled - ourDistance) >= 0.0;

      // Linearly combine the low, mid, and high colors according to the
      // original low, mid, and high components.
      showingUnscaledColor = lowColor * new_currentDataUnscaled.x +
                             midColor * new_currentDataUnscaled.y +
                             highColor * new_currentDataUnscaled.z;

      // Re-scale the color by the maximum component.
      float showingUnscaledMax = max(showingUnscaledColor.x, max(showingUnscaledColor.y, showingUnscaledColor.z));
      showingUnscaledColor = showingUnscaledColor / showingUnscaledMax;
      showingUnscaledColor.w = 1.0;
    }

    // Draw the axes color as the lowest item on the screen.
    // TODO(owilliams): The "4" in this line makes sure the axis gets
    // rendered even when the waveform is fairly short.  Really this
    // value should be based on the size of the widget.
    if (abs(framebufferSize.y / 2 - pixel.y) <= 4) {
      outputColor.xyz = mix(outputColor.xyz, axesColor.xyz, axesColor.w);
      outputColor.w = 1.0;
    }

    if (showingUnscaled) {
      float alpha = 0.4;
      outputColor.xyz = mix(outputColor.xyz, showingUnscaledColor.xyz, alpha);
      outputColor.w = 1.0;
    }

    if (showing) {
      float alpha = 0.8;
      outputColor.xyz = mix(outputColor.xyz, showingColor.xyz, alpha);
      outputColor.w = 1.0;
    }
    gl_FragColor = outputColor;

}
