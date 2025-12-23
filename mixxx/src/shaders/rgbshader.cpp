#include "shaders/rgbshader.h"

using namespace mixxx;

void RGBShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
uniform highp mat4 matrix;
attribute highp vec4 position; // use vec4 here (will be padded) for matrix multiplication
attribute highp vec3 color;
varying highp vec3 vColor;
void main()
{
    vColor = color;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
varying highp vec3 vColor;
void main()
{
    gl_FragColor = vec4(vColor, 1.0);
}
)--");

    load(vertexShaderCode, fragmentShaderCode);

    m_matrixLocation = uniformLocation("matrix");
    m_positionLocation = attributeLocation("position");
    m_colorLocation = attributeLocation("color");
}
