#version 120

uniform int viewportWidth;
uniform int viewportHeigth;

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

vec4 getWaveformData(float index)
{
    vec2 uv_data;
    uv_data.y = floor(index / float(textureStride));
    uv_data.x = floor(index - uv_data.y * float(textureStride));
    // Divide again to convert to normalized UV coordinates.
    return texture2D(waveformDataTexture, uv_data / float(textureStride));
}

void main(void)
{
    vec2 uv = gl_TexCoord[0].st;

    float new_currentIndex = floor(firstVisualIndex + uv.x * (lastVisualIndex - firstVisualIndex))*2;
    if (uv.y > 0.5) {
        new_currentIndex += 1;
    }

    if (new_currentIndex < 0 || new_currentIndex > waveformLength - 1) {
      gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
      return;
    }

    vec4 new_currentData = getWaveformData(new_currentIndex);

    new_currentData *= allGain;
    new_currentData.x *= lowGain;
    new_currentData.y *= midGain;
    new_currentData.z *= highGain;

    //(vrince) debug see pre-computed signal
    //gl_FragColor = rawFiltredSignal;
    //return;

    vec4 outputColor = vec4(0.0, 0.0, 0.0, 0.0);

    float ourDistance = abs((uv.y - 0.5) * 2.0);
    vec4 signalDistance = new_currentData - ourDistance;

    if (signalDistance.x > 0.0) {
      outputColor.x += lowColor.x;
      outputColor.y += lowColor.y;
      outputColor.z += lowColor.z;
      outputColor.w = 0.8;
    }

    if (signalDistance.y > 0.0) {
      outputColor.x += midColor.x;
      outputColor.y += midColor.y;
      outputColor.z += midColor.z;
      outputColor.w = 0.85;
    }

    if (signalDistance.z > 0.0) {
      outputColor.x += highColor.x;
      outputColor.y += highColor.y;
      outputColor.z += highColor.z;
      outputColor.w = 0.9;
    }

    /*
    vec4 distanceToRigthSignal = 0.5 - uv.y - 0.5 *texture2D(signalTexture,vec2(uv.x,0.25));
    vec4 distanceToLeftSignal = uv.y - 0.5 * texture2D(signalTexture,vec2(uv.x,0.75)) - 0.5;

    if (distanceToRigthSignal.x < 0.0 && distanceToLeftSignal.x < 0.0)
        outputColor += lowColor;

    if( distanceToRigthSignal.y < 0.0 && distanceToLeftSignal.y < 0.0)
        outputColor += midColor;

    if( distanceToRigthSignal.z < 0.0 && distanceToLeftSignal.z < 0.0)
        outputColor += highColor;
    */
    gl_FragColor = outputColor;
    return;

    /*
    uv.y = 0.25;
    vec4 signalRigth = texture2D(signalTexture,uv);


    vec4 outputColor = vec4(0.0,0.0,0.0,0.0);
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
        outputColorColor = mix( outputColorColor, low, clamp(accumulatedData.x,0.1f,0.9f));
    if( accumulatedData.y > 0)
        outputColorColor = mix( outputColorColor, mid, clamp(accumulatedData.y,0.1f,0.9f));
    if( accumulatedData.z > 0)
        outputColorColor = mix( outputColorColor, high, clamp(accumulatedData.z,0.1f,0.9f));

    gl_FragColor = outputColorColor;
    return;
    */
}
