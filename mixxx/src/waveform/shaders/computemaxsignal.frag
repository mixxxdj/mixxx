#extension GL_EXT_gpu_shader4 : enable

uniform int waveformLength;
uniform int textureSize;
uniform int textureStride;
uniform float indexPosition;
uniform int zoomFactor;
uniform int width;
uniform int signalFrameBufferRatio;

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

    float currentIndex = indexPosition + 2.0*(uv.x - 0.5) * visualSampleRange;

    if( currentIndex<0.0 || currentIndex > waveformLength)
        discard;

    int nearestCurrentIndex = int(floor(currentIndex));
    currentIndex -= float(nearestCurrentIndex%(2*visualSamplePerPixel));

    float previousIndex = currentIndex - 2.0*float(visualSamplePerPixel);
    float nextIndex = currentIndex + 2.0*float(visualSamplePerPixel);

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
        maxSignalData.x = max( maxSignalData.x, currentData.x);
        maxSignalData.y = max( maxSignalData.y, currentData.y);
        maxSignalData.z = max( maxSignalData.z, currentData.z);
        maxSignalData.a = max( maxSignalData.a, currentData.a);
    }

    gl_FragColor = maxSignalData;
    return;
}
