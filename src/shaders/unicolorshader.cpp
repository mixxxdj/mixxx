#include "shaders/unicolorshader.h"

using namespace mixxx;

void UnicolorShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
uniform highp mat4 matrix;
in highp vec4 position; // use vec4 here (will be padded) for matrix multiplication
out highp vec4 fragColor;
void main()
{
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
uniform highp vec4 color;
out highp vec4 fragColor;
void main()
{
    fragColor = color;
}
)--");

    load(vertexShaderCode, fragmentShaderCode);

    m_matrixLocation = uniformLocation("matrix");
    m_positionLocation = attributeLocation("position");
    m_colorLocation = uniformLocation("color");
}
