//#version 100
#extension GL_EXT_gpu_shader4 : enable

uniform int waveformLength;
uniform int textureSize;
uniform int textureStride;
uniform float indexPosition;

uniform int zoomFactor;

uniform int viewportWidth;
uniform int viewportHeigth;

uniform vec4 lowColor;
uniform vec4 midColor;
uniform vec4 highColor;

uniform sampler2D waveformDataTexture;

vec4 getWaveformData( float index)
{
    vec2 uv_data;
    uv_data.y = floor( index / float(textureStride));
    uv_data.x = floor( index - uv_data.y * float(textureStride));
    return texture2D( waveformDataTexture, uv_data / float(textureStride));
}

vec4 getWaveformData_linearInterpolation( float index)
{
    float ratio = index - floor(index);

    float firstIndex = floor(index);
    float secondIndex = index + 2.0;

    vec2 uv_data_first;
    uv_data_first.y = floor( firstIndex / float(textureStride));
    uv_data_first.x = floor( firstIndex - uv_data_first.y * float(textureStride));

    vec2 uv_data_second;
    uv_data_second.y = floor( secondIndex / float(textureStride));
    uv_data_second.x = floor( secondIndex - uv_data_second.y * float(textureStride));

    return mix( texture2D( waveformDataTexture, uv_data_first / float(textureStride)),
                texture2D( waveformDataTexture, uv_data_second / float(textureStride)),
                ratio);
}

#define texel_x 1.0/viewportWidth

void main(void)
{
    ///////////////////

    vec2 uv = gl_TexCoord[0].st;

    //gl_FragColor = vec4( uv.x, uv.y, 0.0, 1.0);
    //return;

    //gl_FragColor = texture2D( waveformDataTexture, uv);
    //return;

    int visualSampleRange = zoomFactor * viewportWidth;
    float pixelWeigth = 0.8f / float(zoomFactor);

    float currentIndex = indexPosition + 2.0*(uv.x - 0.5) * visualSampleRange;
    int nearestCurrentIndex = floor(currentIndex);
    currentIndex -= float(nearestCurrentIndex%(2*zoomFactor));

    float previousIndex = currentIndex - float(2*zoomFactor);
    float nextIndex = currentIndex + float(2*zoomFactor);


    //gl_FragColor = vec4( (currentIndex - firstIndex)/(2.0*(float)displayRange), firstIndex/float(waveformLength), 1.0, 1.0);
    //return;

    //gl_FragColor = getWaveformData(currentIndex);
    //return;

    vec4 outputColor = vec4(0.0,0.0,0.0,0.0);

    //float firstPixelPosition = currentIndex - pixelRange/2.0;
    //float lastPixelPosition = firstPixelPosition + pixelRange;

    float firstPixelPosition = previousIndex;
    float lastPixelPosition = nextIndex;

    vec3 accumulatedData = vec3(0.0,0.0,0.0);
    //vec3 meanData = vec3(0.0);

    for( float i = firstPixelPosition; i < lastPixelPosition; i += 2.0) {
        vec4 currentData;
        if( uv.y > 0.5) {
            //currentData = getWaveformData_linearInterpolation(i);
            currentData = getWaveformData(i);

            if( currentData.x > uv.y-0.5) //low
                accumulatedData.x += pixelWeigth;
            if( currentData.y > uv.y-0.5) //Mid
                accumulatedData.y += pixelWeigth;
            if( currentData.z > uv.y-0.5) //High
                accumulatedData.z += pixelWeigth;
        }
        else {
            currentData = getWaveformData(i+1);

            if( currentData.x > -uv.y+0.5) //low
                accumulatedData.x += pixelWeigth;
            if( currentData.y > -uv.y+0.5) //Mid
                accumulatedData.y += pixelWeigth;
            if( currentData.z > -uv.y+0.5) //High
                accumulatedData.z += pixelWeigth;
        }
    }

    vec4 low = lowColor;
    //low.a = 0.25+3.0*abs(uv.y-0.5)/accumulatedData.x;
    low.a = accumulatedData.x;

    vec4 mid = midColor;
    //mid.a = 0.5+3.0*abs(uv.y-0.5)/accumulatedData.y;
    mid.a = accumulatedData.y;

    vec4 high = highColor;
    //high.a = clamp(0.25+(1.0-abs(uv.y-0.5)-0.1*accumulatedData.z)/accumulatedData.z,0.f,1.f);
    high.a = accumulatedData.z;

    if( accumulatedData.x > 0)
        outputColor = mix( outputColor, low, clamp(accumulatedData.x,0.1f,0.9f));
    if( accumulatedData.y > 0)
        outputColor = mix( outputColor, mid, clamp(accumulatedData.y,0.1f,0.9f));
    if( accumulatedData.z > 0)
        outputColor = mix( outputColor, high, clamp(accumulatedData.z,0.1f,0.9f));

    gl_FragColor = outputColor;
    return;
}
