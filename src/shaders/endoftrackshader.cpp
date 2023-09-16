#include "shaders/endoftrackshader.h"

using namespace mixxx;

void EndOfTrackShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
attribute highp vec4 position;
attribute highp float gradient;
varying highp float vgradient;
void main()
{
    vgradient = gradient;
    gl_Position = position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
uniform highp vec4 color;
varying highp float vgradient;
void main()
{
    float minAlpha = 0.5 * color.w;
    float maxAlpha = 0.83 * color.w;
    gl_FragColor = vec4(color.xyz, mix(minAlpha, maxAlpha, max(0.,vgradient)));
}
)--");

    load(vertexShaderCode, fragmentShaderCode);
}
