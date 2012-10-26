#extension GL_EXT_gpu_shader4 : enable

uniform int waveformLength;
uniform int textureSize;
uniform int textureStride;
uniform float playPosition;
uniform int zoomFactor;
uniform int width;
uniform int signalFrameBufferRatio;
uniform float gain;
uniform float lowGain;
uniform float midGain;
uniform float highGain;

uniform sampler2D waveformDataTexture;

vec4 getWaveformData( float index)
{
    vec2 uv_data;
    uv_data.y = floor( index / float(textureStride));
    uv_data.x = floor( index - uv_data.y * float(textureStride));
    return texture2D( waveformDataTexture, uv_data / float(textureStride));
}

void main(void)
{
    vec2 uv = gl_TexCoord[0].st;

    int visualSamplePerPixel = signalFrameBufferRatio * zoomFactor;
    float visualSampleRange = float(visualSamplePerPixel * width);

    float bufferPositionRange = visualSampleRange / float(waveformLength);
    float currentPosition = playPosition + 2.0*(uv.x - 0.5) * bufferPositionRange ;

    if( currentPosition < 0.0 || currentPosition > 1.0)
        discard;

    int nearestCurrentIndex = int(floor(currentPosition*float(waveformLength)+0.5));
    float currentIndex = float(nearestCurrentIndex)  - float(nearestCurrentIndex%(2*visualSamplePerPixel));

    float previousIndex = currentIndex - 1.0*float(visualSamplePerPixel);
    float nextIndex = currentIndex + 1.0*float(visualSamplePerPixel);

    vec4 outputColor = vec4(0.0,0.0,0.0,0.0);

    float firstPixelPosition = previousIndex;
    float lastPixelPosition = nextIndex;

    vec4 maxSignalData = vec4(0.0,0.0,0.0,1.0);

    if( uv.y > 0.5) { //rigth
        firstPixelPosition += 1.0;
        lastPixelPosition += 1.0;
    }

    for( float i = firstPixelPosition; i < lastPixelPosition; i += 2.0) {
        vec4 currentData = getWaveformData(i);
        maxSignalData.x = max( maxSignalData.x, (2.0*gain+1.0)*currentData.x);
        maxSignalData.y = max( maxSignalData.y, (2.0*gain+1.0)*currentData.y);
        maxSignalData.z = max( maxSignalData.z, (2.0*gain+1.0)*currentData.z);
        maxSignalData.a = max( maxSignalData.a, (2.0*gain+1.0)*currentData.a);
    }

    gl_FragColor = maxSignalData;
    return;
}
