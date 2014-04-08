uniform int viewportWidth;
uniform int viewportHeigth;

uniform vec4 lowColor;
uniform vec4 midColor;
uniform vec4 highColor;

uniform int textureSize;
uniform sampler2D signalTexture;

void main(void)
{
    vec2 uv = gl_TexCoord[0].st;

    vec4 rawFiltredSignal = texture2D(signalTexture, uv);

    //(vrince) debug see pre-computed signal
    //gl_FragColor = rawFiltredSignal;
    //return;

    vec4 outputColor = vec4(0.0, 0.0, 0.0, 0.0);

    float ourDistance = abs((uv.y - 0.5) * 2.0);
    vec4 signalDistance = rawFiltredSignal - ourDistance;

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
      outputColor.w = 0.8;
    }

    if (signalDistance.z > 0.0) {
      outputColor.x += highColor.x;
      outputColor.y += highColor.y;
      outputColor.z += highColor.z;
      outputColor.w = 0.8;
    }

    gl_FragColor = outputColor;
    return;
}
