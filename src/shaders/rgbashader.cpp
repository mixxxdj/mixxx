#include "shaders/rgbashader.h"

using namespace mixxx;

void RGBAShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
uniform highp mat4 matrix;
in highp vec4 position; // use vec4 here (will be padded) for matrix multiplication
in highp vec4 color;
in highp vec4 vColor;
out highp vec4 fragColor;
void main()
{
    vColor = color;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
in highp vec4 vColor;
out highp vec4 fragColor;
void main()
{
    fragColor = vColor;
}
)--");

    load(vertexShaderCode, fragmentShaderCode);

    m_matrixLocation = uniformLocation("matrix");
    m_positionLocation = attributeLocation("position");
    m_colorLocation = attributeLocation("color");
}
