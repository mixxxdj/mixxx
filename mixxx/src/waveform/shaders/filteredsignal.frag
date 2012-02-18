//#version 100
#extension GL_EXT_gpu_shader4 : enable

uniform int waveformLength;
uniform int textureSize;
uniform int textureStride;
uniform float indexPosition;

uniform int zoomFactor;

uniform int viewportWidth;
uniform int viewportHeigth;

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
    float pixelWeigth = 300.0 / float(visualSampleRange);

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

    for( float i = firstPixelPosition; i < lastPixelPosition; i += 2.0)
    {
        vec4 currentData;
        if( uv.y > 0.5)
        {
            //currentData = getWaveformData_linearInterpolation(i);
            currentData = getWaveformData(i);

            if( currentData.x > uv.y-0.5) //low
                accumulatedData.x += pixelWeigth;
            if( currentData.y > uv.y-0.5) //Mid
                accumulatedData.y += pixelWeigth;
            if( currentData.z > uv.y-0.5) //High
                accumulatedData.z += pixelWeigth;
        }
        else
        {
            currentData = getWaveformData(i+1);

            if( currentData.x > -uv.y+0.5) //low
                accumulatedData.x += pixelWeigth;
            if( currentData.y > -uv.y+0.5) //Mid
                accumulatedData.y += pixelWeigth;
            if( currentData.z > -uv.y+0.5) //High
                accumulatedData.z += pixelWeigth;
        }
    }

    if( accumulatedData.x > uv.y-0.5)
        outputColor += vec4(1.0,0.0,0.0,0.25+0.75*accumulatedData.x);

    outputColor += vec4(0.0,accumulatedData.y,0.0,accumulatedData.y);
    outputColor += vec4(0.0,0.0,accumulatedData.z,accumulatedData.z);

    gl_FragColor = outputColor;
    return;

    //SOME TEST
    //////////////////////////////////////////////////////////

    //to texture1D point
    //float u = floor(currentIndex/textureStride);
    //float v = floor(currentIndex - u * textureStride);

    //vec2 data_uv = vec2(v,u);

    //gl_FragColor = vec4( u / textureStride, v / textureStride, 1.0, 1.0);
    //return;

    //vec2 uv_data;
    //uv_data.y = floor( currentIndex / float(textureStride));
    //uv_data.x = currentIndex - uv_data.y * float(textureStride);
    //vec4 data = texture2D( waveformDataTexture, uv_data / float(textureStride));

    gl_FragColor = getWaveformData(currentIndex);
    return;

    //vec4 data = getWaveformData(currentIndex);

    //gl_FragColor = vec4( data.x, data.y, data.z, 1.0);
    //return;

    vec4 data;
    if( data.x < uv.y)
        outputColor += vec4( 1.0, 0.0, 0.0, 0.33);

    if( data.y < uv.y)
        outputColor += vec4( 0.0, 1.0, 0.0, 0.33);

    if( data.z < uv.y)
        outputColor += vec4( 0.0, 0.0, 1.0, 0.33);

    gl_FragColor = outputColor;

    //gl_FragColor = vec4( 0.5 + 0.5*t.x, 0.5 + 0.5*t.y, t.z, 1.0);

    //gl_FragColor = t;

    //vec4 coord = 0.1*gl_FragCoord;

    //gl_FragColor = coord;
}
