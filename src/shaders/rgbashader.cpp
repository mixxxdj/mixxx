#include "shaders/rgbashader.h"

using namespace mixxx;

void RGBAShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
uniform mat4 matrix;
attribute highp vec4 position;
attribute highp vec4 color;
varying highp vec4 vcolor;
void main()
{
    vcolor = color;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
varying highp vec4 vcolor;
void main()
{
    gl_FragColor = vcolor;
}
)--");

    load(vertexShaderCode, fragmentShaderCode);
}
