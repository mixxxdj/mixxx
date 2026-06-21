#include "shaders/rgbshader.h"

using namespace mixxx;

void RGBShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
#version 310 es
precision mediump float;
uniform highp mat4 matrix;
in highp vec4 position; // use vec4 here (will be padded) for matrix multiplication
in highp vec3 color;
out highp vec3 vColor;
void main()
{
    vColor = color;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
#version 310 es
precision mediump float;
in highp vec3 vColor;
out highp vec4 fragColor;
void main()
{
    fragColor = vec4(vColor, 1.0);
}
)--");

    load(vertexShaderCode, fragmentShaderCode);

    m_matrixLocation = uniformLocation("matrix");
    m_positionLocation = attributeLocation("position");
    m_colorLocation = attributeLocation("color");
}
