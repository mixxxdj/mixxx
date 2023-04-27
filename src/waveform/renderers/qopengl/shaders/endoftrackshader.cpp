#include "waveform/renderers/qopengl/shaders/endoftrackshader.h"

using namespace qopengl;

void EndOfTrackShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
attribute vec4 position;
attribute float gradient;
varying float vgradient;
void main()
{
    vgradient = gradient;
    gl_Position = position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
uniform vec4 color;
varying float vgradient;
void main()
{
    float minAlpha = 0.5 * color.w;
    float maxAlpha = 0.83 * color.w;
    gl_FragColor = vec4(color.xyz, mix(minAlpha, maxAlpha, max(0.,vgradient)));
}
)--");

    load(vertexShaderCode, fragmentShaderCode);
}
