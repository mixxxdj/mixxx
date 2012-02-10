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

    float currentIndex = indexPosition + (uv.x - 0.5) * 2.0 * displayRange;

    //gl_FragColor = vec4( (currentIndex - firstIndex)/(2.0*(float)displayRange), firstIndex/float(waveformLength), 1.0, 1.0);
    //return;

    //gl_FragColor = getWaveformData(currentIndex);
    //return;

    vec4 outputColor = vec4(0.0,0.0,0.0,0.0);

    float firstPixelPosition = currentIndex - pixelRange;
    float lastPixelPosition = currentIndex + pixelRange;

    float accumulatedWaveformData[3] = {0.0,0.0,0.0};
    for( float i = firstPixelPosition; i <= lastPixelPosition; i += 2)
    {
        vec4 currentData = getWaveformData(i);
        if( currentData.x > uv.y) //low
            accumulatedWaveformData[0] += pixelWeigth;
        if( currentData.y > uv.y) //Mid
            accumulatedWaveformData[1] += pixelWeigth;
        if( currentData.z > uv.y) //High
            accumulatedWaveformData[2] += pixelWeigth;
        //if( currentData.a > uv.y) //All
        //    accumulatedWaveformData += vec4(1.0,1.0,1.0,pixelWeigth);
    }

    outputColor = accumulatedWaveformData[0] * vec4(1.0,0.0,0.0,0.5);

    gl_FragColor = outputColor;
    return;

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
