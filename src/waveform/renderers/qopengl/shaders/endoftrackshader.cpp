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
    const float baseTransparency = 0.5;
    const float additionalTransparency = 0.33;
    gl_FragColor = vec4(color.xyz, color.w * (baseTransparency + additionalTransparency * max(0.,vgradient)));
}
)--");

    load(vertexShaderCode, fragmentShaderCode);
}
