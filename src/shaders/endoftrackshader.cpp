#include "shaders/endoftrackshader.h"

using namespace mixxx;

void EndOfTrackShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
in highp vec4 position; // use vec4 here (will be padded) to assign directly to gl_Position
in highp float gradient;
in highp float vgradient;
out highp vec4 fragColor;
void main()
{
    vgradient = gradient;
    gl_Position = position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
uniform highp vec4 color;
in highp float vgradient;
out highp vec4 fragColor;
void main()
{
    highp float minAlpha = 0.5 * color.w;
    highp float maxAlpha = 0.83 * color.w;
    fragColor = vec4(color.xyz, mix(minAlpha, maxAlpha, max(0.0, vgradient)));
}
)--");

    load(vertexShaderCode, fragmentShaderCode);

    m_positionLocation = attributeLocation("position");
    m_gradientLocation = attributeLocation("gradient");
    m_colorLocation = uniformLocation("color");
}
