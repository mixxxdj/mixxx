#include "shaders/rgbashader.h"

using namespace mixxx;

void RGBAShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
uniform mat4 matrix;
attribute vec4 position;
attribute vec4 color;
varying vec4 vcolor;
void main()
{
    vcolor = color;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
varying vec4 vcolor;
void main()
{
    gl_FragColor = vcolor;
}
)--");

    load(vertexShaderCode, fragmentShaderCode);
}
