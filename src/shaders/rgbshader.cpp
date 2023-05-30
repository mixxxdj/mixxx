#include "shaders/rgbshader.h"

using namespace mixxx;

void RGBShader::init() {
    QString vertexShaderCode = QStringLiteral(R"--(
uniform mat4 matrix;
attribute vec4 position;
attribute vec3 color;
varying vec3 vcolor;
void main()
{
    vcolor = color;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
varying vec3 vcolor;
void main()
{
    gl_FragColor = vec4(vcolor,1.0);
}
)--");

    load(vertexShaderCode, fragmentShaderCode);
}
