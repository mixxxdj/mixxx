//#version 100
#extension GL_EXT_gpu_shader4 : enable

uniform int waveformLength;
uniform int textureSize;
uniform int textureStride;
uniform float indexPosition;
uniform float displayRange;

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

void main(void)
{
    float pixelRange = float(displayRange) / float(viewportWidth);
    float pixelWeigth = 1.0 / pixelRange;

    ///////////////////

    vec2 uv = gl_TexCoord[0].st;

    //gl_FragColor = vec4( uv.x, uv.y, 0.0, 1.0);
    //return;

    //gl_FragColor = texture2D( waveformDataTexture, uv);
    //return;

    float currentIndex = indexPosition + 2.0*(uv.x - 0.5) * displayRange;

    //place current index to left channel
    int diff = int(floor(currentIndex+0.5));
    currentIndex -= float(diff%2);

    //gl_FragColor = vec4( (currentIndex - firstIndex)/(2.0*(float)displayRange), firstIndex/float(waveformLength), 1.0, 1.0);
    //return;

    //gl_FragColor = getWaveformData(currentIndex);
    //return;

    vec4 outputColor = vec4(0.0,0.0,0.0,0.0);

    float firstPixelPosition = currentIndex - pixelRange/2.0;
    float lastPixelPosition = currentIndex + pixelRange/2.0;

    vec3 accumulatedData = vec3(0.0,0.0,0.0);

    for( float i = firstPixelPosition; i <= lastPixelPosition; i += 2)
    {
        vec4 currentData;
        if( uv.y > 0.5)
        {
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

    outputColor += vec4(1.0,0.0,0.0,accumulatedData.x) * accumulatedData.x;
    outputColor += vec4(0.0,1.0,0.0,accumulatedData.y) * accumulatedData.y;
    outputColor += vec4(0.0,0.0,1.0,accumulatedData.z) * accumulatedData.z;

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
