uniform highp vec2 framebufferSize;
uniform highp vec4 axesColor;
uniform highp vec4 lowColor;
uniform highp vec4 midColor;
uniform highp vec4 highColor;

uniform int waveformLength;
uniform int textureSize;
uniform int textureStride;

uniform highp float allGain;
uniform highp float lowGain;
uniform highp float midGain;
uniform highp float highGain;
uniform highp float firstVisualIndex;
uniform highp float lastVisualIndex;

uniform sampler2D waveformDataTexture;

vec4 getWaveformData(float index) {
    highp vec2 uv_data;
    uv_data.y = floor(index / float(textureStride));
    uv_data.x = floor(index - uv_data.y * float(textureStride));
    // Divide again to convert to normalized UV coordinates.
    return texture2D(waveformDataTexture, uv_data / float(textureStride));
}

void main(void) {
    highp vec2 uv = gl_TexCoord[0].st;
    highp vec4 pixel = gl_FragCoord;

    highp float new_currentIndex =
            floor(firstVisualIndex +
                    uv.x * (lastVisualIndex - firstVisualIndex)) *
            2.0;

    // Texture coordinates put (0,0) at the bottom left, so show the right
    // channel if we are in the bottom half.
    if (uv.y < 0.5) {
        new_currentIndex += 1.0;
    }

    highp vec4 outputColor = vec4(0.0, 0.0, 0.0, 0.0);
    bool lowShowing = false;
    bool midShowing = false;
    bool highShowing = false;
    bool maxShowingUnscaled = false;
    // We don't exit early if the waveform data is not valid because we may want
    // to show other things (e.g. the axes lines) even when we are on a pixel
    // that does not have valid waveform data.
    if (new_currentIndex >= 0.0 && new_currentIndex <= float(waveformLength - 1)) {
        highp vec4 new_currentDataUnscaled = getWaveformData(new_currentIndex) * allGain;
        highp vec4 new_currentData = new_currentDataUnscaled;

        new_currentData.x *= lowGain;
        new_currentData.y *= midGain;
        new_currentData.z *= highGain;

        //(vrince) debug see pre-computed signal
        // gl_FragColor = new_currentData;
        // return;

        // Represents the [-1, 1] distance of this pixel. Subtracting this from
        // the signal data in new_currentData, we can tell if a signal band should
        // show in this pixel if the component is > 0.
        highp float ourDistance = abs((uv.y - 0.5) * 2.0);

        highp vec4 signalDistance = new_currentData - ourDistance;
        lowShowing = signalDistance.x >= 0.0;
        midShowing = signalDistance.y >= 0.0;
        highShowing = signalDistance.z >= 0.0;
        maxShowingUnscaled = new_currentDataUnscaled.x - ourDistance >= 0.0 ||
                new_currentDataUnscaled.y - ourDistance >= 0.0 ||
                new_currentDataUnscaled.z - ourDistance >= 0.0;
    }

    // Draw the axes color as the lowest item on the screen.
    // TODO(owilliams): The "4" in this line makes sure the axis gets
    // rendered even when the waveform is fairly short.  Really this
    // value should be based on the size of the widget.
    if (abs(framebufferSize.y / 2.0 - pixel.y) <= 4.0) {
        outputColor = axesColor;
    } else if (maxShowingUnscaled) {
        outputColor.xyz = axesColor.xyz;
        outputColor.w = axesColor.w * 0.2;
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

    /*
    highp vec4 distanceToRightSignal = 0.5 - uv.y - 0.5 *texture2D(signalTexture,vec2(uv.x,0.25));
    highp vec4 distanceToLeftSignal = uv.y - 0.5 * texture2D(signalTexture,vec2(uv.x,0.75)) - 0.5;

    if (distanceToRightSignal.x < 0.0 && distanceToLeftSignal.x < 0.0)
        outputColor += lowColor;

    if( distanceToRightSignal.y < 0.0 && distanceToLeftSignal.y < 0.0)
        outputColor += midColor;

    if( distanceToRightSignal.z < 0.0 && distanceToLeftSignal.z < 0.0)
        outputColor += highColor;
    */
    gl_FragColor = outputColor;
    return;

    /*
    uv.y = 0.25;
    highp vec4 signalRight = texture2D(signalTexture,uv);


    highp vec4 outputColor = vec4(0.0,0.0,0.0,0.0);
    highp vec3 accumulatedData = vec3(0.0,0.0,0.0);
    //vec3 meanData = vec3(0.0);

    for(highp float i = firstPixelPosition; i < lastPixelPosition; i += 2.0) {
        highp vec4 currentData;
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

    highp vec4 low = lowColor;
    //low.a = 0.25+3.0*abs(uv.y-0.5)/accumulatedData.x;
    low.a = accumulatedData.x;

    highp vec4 mid = midColor;
    //mid.a = 0.5+3.0*abs(uv.y-0.5)/accumulatedData.y;
    mid.a = accumulatedData.y;

    highp vec4 high = highColor;
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
