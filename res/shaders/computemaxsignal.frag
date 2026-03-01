#extension GL_EXT_gpu_shader4 : enable

uniform int waveformLength;
uniform int textureSize;
uniform int textureStride;
uniform highp float playPosition;
uniform int zoomFactor;
uniform int width;
uniform int signalFrameBufferRatio;
uniform highp float allGain;
uniform highp float lowGain;
uniform highp float midGain;
uniform highp float highGain;
uniform highp float firstVisualIndex;
uniform highp float lastVisualIndex;

uniform sampler2D waveformDataTexture;

highp vec4 getWaveformData(highp float index) {
    vec2 uv_data;
    uv_data.y = floor(index / float(textureStride));
    uv_data.x = floor(index - uv_data.y * float(textureStride));
    // Divide again to convert to normalized UV coordinates.
    return texture2D(waveformDataTexture, uv_data / float(textureStride));
}

void main(void)
{
    highp vec2 uv = gl_TexCoord[0].st;

    highp float new_currentIndex =
            floor(firstVisualIndex +
                    uv.x * (lastVisualIndex - firstVisualIndex)) *
            2.0;
    if (uv.y > 0.5) {
        new_currentIndex += 1;
    }

    if (new_currentIndex < 0.0 || new_currentIndex > float(waveformLength - 1)) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    }

    highp vec4 new_currentData = getWaveformData(new_currentIndex);

    new_currentData *= allGain;
    new_currentData.x *= lowGain;
    new_currentData.y *= midGain;
    new_currentData.z *= highGain;

    gl_FragColor = new_currentData;
    return;


    int visualSamplePerPixel = zoomFactor;
    highp float visualSampleRange = float(visualSamplePerPixel * width);

    highp float bufferPositionRange = visualSampleRange / float(waveformLength);

    // uv.x is percentage across the texture of this pixel. subtract 0.5 to
    // transform to [-0.5, 0.5], scale to [-1, 1].
    float currentPosition = playPosition + 2.0*(uv.x - 0.5) * bufferPositionRange ;

    // Before or after the waveform.
    if (currentPosition < 0.0 || currentPosition > 1.0) {
      gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
      return;
    }

    // Round to nearest index.
    int nearestCurrentIndex = int(floor(currentPosition*float(waveformLength)+0.5));

    float currentIndex = float(nearestCurrentIndex);

    // This prevents wiggling by locking the current index to fixed multiples of
    // visualSamplePerPixel.
    currentIndex -= mod(currentIndex, 2*visualSamplePerPixel);

    float previousIndex = currentIndex - 0.5*float(visualSamplePerPixel);
    float nextIndex = currentIndex + 0.5*float(visualSamplePerPixel);

    int firstPixelPosition = int(previousIndex);
    if (mod(firstPixelPosition, 2) == 0) {
      firstPixelPosition -= 1;
    }

    int lastPixelPosition = int(nextIndex + 0.5);
    if (mod(lastPixelPosition, 2) == 0) {
      lastPixelPosition -= 1;
    }

    vec4 maxSignalData = vec4(0.0, 0.0, 0.0, 0.0);

    // TODO(rryan): Check we are correct here on l vs. r
    // If we are in the bottom half, we use the right channel by adding 1 to the
    // position offset. The loop below will then sample across the right
    // channels instead of the left.
    if (uv.y > 0.5) {
        firstPixelPosition += 1;
        lastPixelPosition += 1;
    }

    for (int i = firstPixelPosition; i < lastPixelPosition; i += 2) {
        vec4 currentData = getWaveformData(i);
        maxSignalData.x = max(maxSignalData.x, currentData.x);
        maxSignalData.y = max(maxSignalData.y, currentData.y);
        maxSignalData.z = max(maxSignalData.z, currentData.z);
        maxSignalData.w = max(maxSignalData.w, currentData.w);
    }

    // Scale the signal from 0 to 1.
    //maxSignalData /= 255.0;
    maxSignalData *= allGain;
    maxSignalData.x *= lowGain;
    maxSignalData.y *= midGain;
    maxSignalData.z *= highGain;

    gl_FragColor = maxSignalData;
    return;
}
