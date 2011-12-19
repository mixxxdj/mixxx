uniform int waveformLength;
uniform int textureSize;
uniform int textureStride;
uniform int indexPosition;
uniform int displayRange;

uniform sampler2D texture;

void main(void)
{
    vec2 uv = gl_TexCoord[0].st;

    //gl_FragColor = vec4( 0.5 + 0.5*uv.x, 0.5 + 0.5*uv.y, 1.0, 1.0);
    //return;

    float firstIndex = float(indexPosition) - float(displayRange);
    float currentIndex = float(indexPosition) + uv.x * float(displayRange);
    float lastIndex = float(indexPosition) + float(displayRange);

    //gl_FragColor = vec4( (currentIndex - firstIndex)/(2.0*(float)displayRange), firstIndex/float(waveformLength), 1.0, 1.0);
    //return;

    //to texture1D point
    float u = floor(currentIndex/textureStride);
    float v = floor(currentIndex - u * textureStride);

    vec2 data_uv = vec2(v,u);

    gl_FragColor = vec4( u / textureStride, v / textureStride, 1.0, 1.0);
    return;

    vec4 data = texture2D( texture, data_uv / textureStride);

    //gl_FragColor = vec4( data.x, data.y, data.z, 1.0);
    //return;

    vec4 outputColor = vec4(0.0,0.0,0.0,0.0);

    if( data.x < uv.y + 0.5)
        outputColor += vec4( 1.0, 0.0, 0.0, 0.33);

    if( data.y < uv.y + 0.5)
        outputColor += vec4( 0.0, 1.0, 0.0, 0.33);

    if( data.z < uv.y + 0.5)
        outputColor += vec4( 0.0, 0.0, 1.0, 0.33);

    gl_FragColor = outputColor;

    //gl_FragColor = vec4( 0.5 + 0.5*t.x, 0.5 + 0.5*t.y, t.z, 1.0);

    //gl_FragColor = t;

    //vec4 coord = 0.1*gl_FragCoord;

    //gl_FragColor = coord;
}
